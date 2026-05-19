#!/usr/bin/env bash
# Helpers compartilhados pelas suites de teste de integracao.
# Pre-requisito: webserv ja esta rodando em 127.0.0.1:8080 e 127.0.0.1:8081
# com a config padrao (config.conf).

BASE_URL="${BASE_URL:-http://127.0.0.1:8080}"
BASE_URL_RO="${BASE_URL_RO:-http://127.0.0.1:8081}"

GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[0;33m"
NC="\033[0m"

PASS_COUNT=0
FAIL_COUNT=0
SUITE_NAME="${SUITE_NAME:-suite}"

pass() {
    PASS_COUNT=$((PASS_COUNT + 1))
    echo -e "  ${GREEN}OK${NC}   $1"
}

fail() {
    FAIL_COUNT=$((FAIL_COUNT + 1))
    echo -e "  ${RED}FAIL${NC} $1"
    [ -n "$2" ] && echo -e "       ${YELLOW}$2${NC}"
}

# Verifica codigo de status. Aceita comando que ja imprime cabecalhos HTTP
# (curl -s -i ou printf | nc).
# Uso: assert_status <nome> <comando> <status_esperado>
assert_status() {
    local name="$1"
    local cmd="$2"
    local expected="$3"
    local out status

    out=$(eval "$cmd" 2>/dev/null)
    status=$(echo "$out" | grep -m1 -oE 'HTTP/1\.[01] [0-9]+' | awk '{print $2}')

    if [ "$status" = "$expected" ]; then
        pass "$name (status=$status)"
    else
        fail "$name" "esperado $expected, recebeu '${status:-<vazio>}'"
    fi
}

# Verifica que um header especifico contem um valor (substring).
# Uso: assert_header <nome> <comando> <header> <substring_esperada>
assert_header() {
    local name="$1"
    local cmd="$2"
    local header="$3"
    local expected="$4"
    local out value

    out=$(eval "$cmd" 2>/dev/null)
    value=$(echo "$out" | grep -i -m1 "^${header}:" | sed -E "s/^[^:]+:[[:space:]]*//" | tr -d '\r')

    if echo "$value" | grep -qF -- "$expected"; then
        pass "$name ($header: $value)"
    else
        fail "$name" "header $header nao contem '$expected' (recebido: '${value:-<ausente>}')"
    fi
}

# Verifica que o body da resposta contem uma substring.
# Uso: assert_body <nome> <comando> <substring_esperada>
assert_body() {
    local name="$1"
    local cmd="$2"
    local expected="$3"
    local out body

    out=$(eval "$cmd" 2>/dev/null)
    # body comeca apos a primeira linha em branco
    body=$(echo "$out" | awk 'BEGIN{h=1} /^\r?$/ && h{h=0;next} !h{print}')

    if echo "$body" | grep -qF -- "$expected"; then
        pass "$name (body contem '$expected')"
    else
        fail "$name" "body nao contem '$expected'"
    fi
}

# Verifica que o servidor responde em <host:port>.
# Uso: ensure_server_up <url>
ensure_server_up() {
    local url="$1"
    if ! curl -s --max-time 2 -o /dev/null "$url"; then
        echo -e "${RED}!! Servidor nao responde em $url. Suba o webserv antes de rodar.${NC}"
        exit 2
    fi
}

suite_header() {
    echo "=============================="
    echo "  $SUITE_NAME"
    echo "=============================="
}

suite_summary() {
    local total=$((PASS_COUNT + FAIL_COUNT))
    echo "------------------------------"
    echo -e "  ${GREEN}${PASS_COUNT}/${total} OK${NC} | ${RED}${FAIL_COUNT} FAIL${NC}"
    [ "$FAIL_COUNT" -eq 0 ]
}
