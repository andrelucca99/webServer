#!/usr/bin/env bash
# Suite: erros HTTP (400, 405, 505)
# Cobertura: validacoes de request line, metodos nao permitidos,
# versao HTTP nao suportada, error_page fallback.

set -u
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SUITE_NAME="ERRORS"
source "$SCRIPT_DIR/lib.sh"

ensure_server_up "$BASE_URL/"
suite_header

# 400 - request line malformada (sem versao)
assert_status "400 request line malformada" \
    "printf 'GET /\r\n\r\n' | nc -w1 127.0.0.1 8080" 400

# 400 - request line sem path
assert_status "400 request line sem path" \
    "printf 'GET HTTP/1.1\r\nHost: localhost\r\n\r\n' | nc -w1 127.0.0.1 8080" 400

# 405 - PUT nao listado em methods
assert_status "405 PUT em /" \
    "curl -s -i -X PUT $BASE_URL/" 405

# 405 - PATCH nao listado em methods
assert_status "405 PATCH em /" \
    "curl -s -i -X PATCH $BASE_URL/" 405

# 405 - POST na porta 8081 (somente GET)
assert_status "405 POST na 8081 (somente GET)" \
    "curl -s -i -X POST $BASE_URL_RO/ -F 'x=@/dev/null'" 405

# 505 - HTTP/2.0 nao suportado
assert_status "505 HTTP/2.0" \
    "printf 'GET / HTTP/2.0\r\nHost: localhost\r\n\r\n' | nc -w1 127.0.0.1 8080" 505

# 505 - HTTP/0.9 nao suportado
assert_status "505 HTTP/0.9" \
    "printf 'GET / HTTP/0.9\r\nHost: localhost\r\n\r\n' | nc -w1 127.0.0.1 8080" 505

# error_page fallback: www/errors/ nao existe -> body inline com codigo
assert_body "404 body fallback contem '404'" \
    "curl -s -i $BASE_URL/inexistente" "404"
assert_body "405 body fallback contem '405'" \
    "curl -s -i -X PUT $BASE_URL/" "405"

suite_summary
