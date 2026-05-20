#!/usr/bin/env bash
# Roda webserv sob valgrind, executa a suite HTTP, manda SIGINT e analisa
# o relatorio. Requer:
#   - valgrind instalado
#   - ./webserv ja buildado (sem sanitizers)
#   - ./parser_smoke_vg ja buildado (sem sanitizers)

set -u
cd "$(dirname "$0")/.."

GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[0;33m"
NC="\033[0m"

LOG_DIR=/tmp/webserv_vg_logs
mkdir -p "$LOG_DIR"

PARSER_VG_LOG="$LOG_DIR/parser_valgrind.log"
PARSER_OUT_LOG="$LOG_DIR/parser_stdout.log"
SERVER_VG_LOG="$LOG_DIR/webserv_valgrind.log"
SERVER_OUT_LOG="$LOG_DIR/webserv_stdout.log"
SUITE_LOG="$LOG_DIR/suite.log"

fail() {
    echo -e "${RED}[FAIL]${NC} $1"
    exit 1
}

require() {
    command -v "$1" >/dev/null 2>&1 || fail "Comando '$1' nao encontrado. Instale com: sudo apt-get install -y $1"
}

require valgrind
require curl

[ -x "./webserv" ]         || fail "./webserv nao existe. Rode 'make' antes."
[ -x "./parser_smoke_vg" ] || fail "./parser_smoke_vg nao existe. Rode 'make valgrind-build' antes."

# ------------------------------------------------------------
# Etapa 1: parser_smoke sob valgrind (memcheck full, exit limpo)
# ------------------------------------------------------------
echo "=============================="
echo "  [1/2] parser_smoke_vg sob valgrind (memcheck)"
echo "=============================="

valgrind \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --error-exitcode=1 \
    --log-file="$PARSER_VG_LOG" \
    ./parser_smoke_vg > "$PARSER_OUT_LOG" 2>&1
parser_rc=$?

tail -20 "$PARSER_OUT_LOG"
echo "--- valgrind summary ---"
grep -E "ERROR SUMMARY|HEAP SUMMARY|in use at exit|definitely lost|indirectly lost|possibly lost|still reachable" \
    "$PARSER_VG_LOG" || true

if [ "$parser_rc" -ne 0 ]; then
    echo -e "${RED}valgrind encontrou erro em parser_smoke_vg (rc=$parser_rc). Log: $PARSER_VG_LOG${NC}"
    exit 1
fi
echo -e "${GREEN}parser_smoke_vg OK${NC}"

# ------------------------------------------------------------
# Etapa 2: webserv sob valgrind + suite HTTP
# ------------------------------------------------------------
echo
echo "=============================="
echo "  [2/2] webserv sob valgrind + suite HTTP"
echo "=============================="

pkill -f ./webserv 2>/dev/null
sleep 0.3

# --child-silent-after-fork=yes: nao reporta para CGI (filho do fork) - eles
#                                executam python3 via execve e nao tem nada
#                                a ver com o nosso codigo.
# --error-exitcode=2: erros memcheck (UAF, OOB, uso de uninit, etc.) fazem
#                     valgrind sair com 2 quando webserv terminar.
# --leak-check=full + --show-leak-kinds=all: pega todas as categorias,
#                                            inclusive still-reachable
#                                            (que ainda assim aparecem
#                                            quando o servidor for morto).
valgrind \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --error-exitcode=2 \
    --child-silent-after-fork=yes \
    --log-file="$SERVER_VG_LOG" \
    ./webserv > "$SERVER_OUT_LOG" 2>&1 &
VG_PID=$!

cleanup() {
    if kill -0 "$VG_PID" 2>/dev/null; then
        # Valgrind nao faz fork: ele exec o programa traced no MESMO processo.
        # Logo $VG_PID e tambem o PID do webserv (do ponto de vista do kernel).
        # SIGTERM aqui faz valgrind sair, emitir leak report e flush do log.
        echo "[*] enviando SIGTERM ao valgrind/webserv PID=$VG_PID"
        kill -TERM "$VG_PID" 2>/dev/null || true
        for _ in $(seq 1 100); do
            kill -0 "$VG_PID" 2>/dev/null || break
            sleep 0.2
        done
        if kill -0 "$VG_PID" 2>/dev/null; then
            echo "[!] valgrind nao saiu em 20s; forcando KILL (leak report sera perdido)."
            kill -KILL "$VG_PID" 2>/dev/null || true
        fi
        wait "$VG_PID" 2>/dev/null || true
    fi
}
trap cleanup EXIT INT TERM

# Espera porta abrir (valgrind = mais lento; ate 30s)
ok_up=
for _ in $(seq 1 150); do
    if curl -s --max-time 1 -o /dev/null http://127.0.0.1:8080/; then
        ok_up=yes
        break
    fi
    sleep 0.2
done

if [ -z "$ok_up" ]; then
    echo -e "${RED}webserv sob valgrind nao respondeu em :8080 dentro do timeout.${NC}"
    echo "--- server stdout ---"
    tail -30 "$SERVER_OUT_LOG"
    echo "--- valgrind log ---"
    tail -30 "$SERVER_VG_LOG"
    exit 1
fi

echo "[*] webserv pronto (sob valgrind). Rodando suite..."
bash tests/run_all.sh 2>&1 | tee "$SUITE_LOG"
suite_rc=${PIPESTATUS[0]}

cleanup
trap - EXIT INT TERM

echo
echo "--- valgrind summary (servidor) ---"
grep -E "in use at exit|total heap usage|All heap blocks were freed|definitely lost|indirectly lost|possibly lost|still reachable|ERROR SUMMARY" \
    "$SERVER_VG_LOG" | tail -15 || true

echo
echo "=============================="
echo "  Resumo"
echo "=============================="

err_count=$(grep -oE "ERROR SUMMARY: [0-9]+ errors" "$SERVER_VG_LOG" | tail -1 \
            | grep -oE "[0-9]+" | head -1)
err_count=${err_count:-?}

# Caso ideal: valgrind reporta "All heap blocks were freed -- no leaks are
# possible". Nesse caso ele SUPRIME o LEAK SUMMARY porque nao ha o que listar.
if grep -q "All heap blocks were freed -- no leaks are possible" "$SERVER_VG_LOG"; then
    in_use=$(grep -oE "in use at exit: [0-9,]+ bytes" "$SERVER_VG_LOG" | tail -1 \
             | sed -E "s/.*: ([0-9,]+) bytes/\1/" | tr -d ',')
    allocs=$(grep -oE "total heap usage: [0-9,]+ allocs" "$SERVER_VG_LOG" | tail -1 \
             | sed -E "s/.*: ([0-9,]+) allocs/\1/" | tr -d ',')
    echo "  in use at exit : ${in_use:-?} bytes"
    echo "  total allocs   : ${allocs:-?} (todas liberadas)"
    echo "  errors         : $err_count"
    echo

    if [ "$err_count" != "0" ]; then
        echo -e "${RED}Valgrind reportou erros (UAF/OOB/uninit). Log: $SERVER_VG_LOG${NC}"
        exit 1
    fi

    echo -e "${GREEN}All heap blocks were freed -- no leaks are possible.${NC}"
    echo "  Logs: $LOG_DIR/"
    exit 0
fi

# Caso "LEAK SUMMARY presente": ha algo still-reachable / lost. Parseia
# bloco a bloco.
extract_bytes() {
    grep -oE "$1: [0-9,]+ bytes" "$SERVER_VG_LOG" | tail -1 \
        | sed -E "s/$1: ([0-9,]+) bytes/\1/" \
        | tr -d ','
}

def_lost=$(extract_bytes "definitely lost"); def_lost=${def_lost:-?}
ind_lost=$(extract_bytes "indirectly lost"); ind_lost=${ind_lost:-?}
pos_lost=$(extract_bytes "possibly lost");   pos_lost=${pos_lost:-?}
still=$(extract_bytes "still reachable");    still=${still:-?}

echo "  definitely lost: $def_lost bytes"
echo "  indirectly lost: $ind_lost bytes"
echo "  possibly lost  : $pos_lost bytes"
echo "  still reachable: $still bytes"
echo "  errors         : $err_count"
echo

if [ "$def_lost" = "?" ] || [ "$err_count" = "?" ]; then
    echo -e "${RED}Nao consegui parsear o relatorio. Log: $SERVER_VG_LOG${NC}"
    exit 1
fi

if [ "$def_lost" != "0" ] || [ "$ind_lost" != "0" ] || [ "$err_count" != "0" ]; then
    echo -e "${RED}Valgrind reportou problemas reais. Log: $SERVER_VG_LOG${NC}"
    exit 1
fi

if [ "$still" != "0" ]; then
    echo -e "${YELLOW}Aviso: still reachable = $still bytes (estado vivo no shutdown).${NC}"
fi

if [ "$suite_rc" -ne 0 ]; then
    echo -e "${YELLOW}Suite HTTP teve falhas (rc=$suite_rc), mas sem erro de memoria.${NC}"
    exit "$suite_rc"
fi

echo -e "${GREEN}Sem erros de memoria reais detectados pelo valgrind.${NC}"
echo "  Logs: $LOG_DIR/"
exit 0
