#!/usr/bin/env bash
# Orquestrador de verificacao de vazamentos.
#
# Etapa 1: roda parser_smoke (binario instrumentado com ASan + LSan + UBSan)
#          que exercita ConfigParser, Tokenizer e HttpRequestParser em caminhos
#          de sucesso e de erro. LSan ativo -> qualquer alocacao nao liberada
#          no exit imprime "ERROR: LeakSanitizer" e o exit code vira != 0.
#
# Etapa 2: sobe ./webserv_san em background e roda a suite HTTP em
#          tests/test_*.sh. ASan continua ativo: use-after-free, OOB,
#          double-free, UB sao detectados durante o trafego. LSan e
#          desabilitado para o servidor (loop infinito sem cleanup ao
#          receber SIGTERM produziria falsos positivos de leak no exit).

set -u
cd "$(dirname "$0")/.."

GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[0;33m"
NC="\033[0m"

LOG_DIR=/tmp/webserv_san_logs
mkdir -p "$LOG_DIR"

PARSER_LOG="$LOG_DIR/parser_smoke.log"
SERVER_LOG="$LOG_DIR/webserv_san.log"
SUITE_LOG="$LOG_DIR/suite.log"

fail() {
    echo -e "${RED}[FAIL]${NC} $1"
    exit 1
}

require_bin() {
    [ -x "./$1" ] || fail "Binario './$1' nao existe. Rode 'make sanitize'."
}

require_bin parser_smoke
require_bin webserv_san

# ------------------------------------------------------------
# Etapa 1: parser_smoke
# ------------------------------------------------------------
echo "=============================="
echo "  [1/2] parser_smoke (ASan + LSan + UBSan)"
echo "=============================="

ASAN_OPTIONS="halt_on_error=1:abort_on_error=0:detect_leaks=1:exitcode=66" \
UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1:exitcode=66" \
LSAN_OPTIONS="exitcode=66" \
    ./parser_smoke 2>&1 | tee "$PARSER_LOG"
rc=${PIPESTATUS[0]}

if [ "$rc" -ne 0 ]; then
    echo -e "${RED}parser_smoke falhou (rc=$rc). Log em $PARSER_LOG.${NC}"
    exit 1
fi

if grep -qE "ERROR: (Address|Leak|Undefined)Sanitizer|runtime error:" "$PARSER_LOG"; then
    echo -e "${RED}Sanitizer reportou problema em parser_smoke. Log em $PARSER_LOG.${NC}"
    exit 1
fi
echo -e "${GREEN}parser_smoke OK${NC}"

# ------------------------------------------------------------
# Etapa 2: webserv_san + suite HTTP
# ------------------------------------------------------------
echo
echo "=============================="
echo "  [2/2] webserv_san + suite HTTP (ASan + UBSan)"
echo "=============================="

# Mata instancias velhas que possam ter sobrado.
pkill -f ./webserv_san 2>/dev/null
pkill -f ./webserv     2>/dev/null
sleep 0.3

# detect_leaks=0: o servidor roda em loop infinito e e morto por sinal;
#                 LSan no exit produziria ruido de "still reachable" que
#                 nao representa vazamento real.
# halt_on_error=1: qualquer UAF/OOB detectado durante o trafego derruba o
#                  processo, e o teste vai falhar (servidor parou de responder).
ASAN_OPTIONS="halt_on_error=1:abort_on_error=0:detect_leaks=0:exitcode=77:log_path=$LOG_DIR/asan_server" \
UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1:exitcode=77:log_path=$LOG_DIR/ubsan_server" \
    ./webserv_san > "$SERVER_LOG" 2>&1 &
SERVER_PID=$!

cleanup() {
    if kill -0 "$SERVER_PID" 2>/dev/null; then
        kill -INT "$SERVER_PID" 2>/dev/null
        sleep 0.3
        kill -KILL "$SERVER_PID" 2>/dev/null || true
        wait "$SERVER_PID" 2>/dev/null || true
    fi
}
trap cleanup EXIT INT TERM

sleep 0.5
if ! kill -0 "$SERVER_PID" 2>/dev/null; then
    echo -e "${RED}webserv_san nao subiu. Server log:${NC}"
    cat "$SERVER_LOG"
    exit 1
fi
echo "[*] webserv_san PID=$SERVER_PID"

# Espera porta abrir (até 5s).
for i in $(seq 1 50); do
    if curl -s --max-time 1 -o /dev/null http://127.0.0.1:8080/; then
        break
    fi
    sleep 0.1
done

if ! curl -s --max-time 2 -o /dev/null http://127.0.0.1:8080/; then
    echo -e "${RED}webserv_san subiu mas nao responde em :8080.${NC}"
    cat "$SERVER_LOG"
    exit 1
fi

bash tests/run_all.sh 2>&1 | tee "$SUITE_LOG"
suite_rc=${PIPESTATUS[0]}

# Da um instante para flushes do ASan caso algo tenha sido reportado em paralelo.
sleep 0.2

# Verifica se ASan/UBSan reportaram problema no log do servidor ou em arquivos
# separados (ASAN log_path=...). Qualquer um reprovar => falha de memoria.
problem_in_server_log=""
if grep -qE "ERROR: AddressSanitizer|ERROR: UndefinedBehaviorSanitizer|runtime error:|AddressSanitizer:" "$SERVER_LOG"; then
    problem_in_server_log=yes
fi

problem_in_asan_files=""
if compgen -G "$LOG_DIR/asan_server.*" > /dev/null || \
   compgen -G "$LOG_DIR/ubsan_server.*" > /dev/null; then
    problem_in_asan_files=yes
fi

echo
echo "=============================="
echo "  Resumo"
echo "=============================="

if [ -n "$problem_in_server_log" ] || [ -n "$problem_in_asan_files" ]; then
    echo -e "${RED}Sanitizer reportou problema no servidor.${NC}"
    echo "  - Server log: $SERVER_LOG"
    [ -n "$problem_in_asan_files" ] && echo "  - ASan/UBSan dumps: $LOG_DIR/asan_server.*, $LOG_DIR/ubsan_server.*"
    exit 1
fi

if [ "$suite_rc" -ne 0 ]; then
    echo -e "${YELLOW}Suite HTTP teve falhas (rc=$suite_rc), mas sem erro de sanitizer.${NC}"
    echo "  Log: $SUITE_LOG"
    exit "$suite_rc"
fi

echo -e "${GREEN}Sem vazamentos / erros de memoria detectados.${NC}"
echo "  Logs: $LOG_DIR/"
exit 0
