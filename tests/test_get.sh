#!/usr/bin/env bash
# Suite: GET
# Cobertura: arquivos estaticos, MIME types, index file, 404, segundo server (porta 8081).

set -u
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SUITE_NAME="GET"
source "$SCRIPT_DIR/lib.sh"

ensure_server_up "$BASE_URL/"
suite_header

# index file: GET / serve www/index.html
assert_status "GET / (index.html)" \
    "curl -s -i $BASE_URL/" 200
assert_header "GET / Content-Type=text/html" \
    "curl -s -i $BASE_URL/" "Content-Type" "text/html"

# arquivo HTML explicito
assert_status "GET /index.html" \
    "curl -s -i $BASE_URL/index.html" 200

# CSS
assert_status "GET /style.css" \
    "curl -s -i $BASE_URL/style.css" 200
assert_header "GET /style.css Content-Type=text/css" \
    "curl -s -i $BASE_URL/style.css" "Content-Type" "text/css"

# JS
assert_status "GET /app.js" \
    "curl -s -i $BASE_URL/app.js" 200
assert_header "GET /app.js Content-Type=application/javascript" \
    "curl -s -i $BASE_URL/app.js" "Content-Type" "application/javascript"

# 404
assert_status "GET /nao-existe.html" \
    "curl -s -i $BASE_URL/nao-existe.html" 404
assert_body "GET 404 body inclui '404'" \
    "curl -s -i $BASE_URL/nao-existe.html" "404"

# Content-Length presente
assert_header "GET / tem Content-Length" \
    "curl -s -i $BASE_URL/" "Content-Length" ""

# Connection: close (servidor sempre fecha)
assert_header "GET / Connection: close" \
    "curl -s -i $BASE_URL/" "Connection" "close"

# Segundo server (porta 8081, somente GET)
assert_status "GET porta 8081" \
    "curl -s -i $BASE_URL_RO/" 200

suite_summary
