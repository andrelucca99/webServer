#!/usr/bin/env bash
# Suite: POST (multipart/form-data + upload de arquivos)
# Cobertura: 201 ao salvar arquivo, sanitizacao de filename (path traversal),
# 400 quando nao e multipart, 413 quando excede client_max_body_size.

set -u
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SUITE_NAME="POST"
source "$SCRIPT_DIR/lib.sh"

ensure_server_up "$BASE_URL/"
suite_header

WWW_DIR="$(cd "$SCRIPT_DIR/../www" && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"; rm -f "$WWW_DIR"/upload_test_*.txt "$WWW_DIR"/hack.txt' EXIT

# fixture
echo "conteudo de upload" > "$TMP/upload_test_simple.txt"

# 1) upload simples
assert_status "POST multipart simples" \
    "curl -s -i -X POST $BASE_URL/ -F 'file=@$TMP/upload_test_simple.txt'" 201

# arquivo realmente foi gravado
if [ -f "$WWW_DIR/upload_test_simple.txt" ]; then
    pass "POST salvou arquivo em www/upload_test_simple.txt"
    grep -q "conteudo de upload" "$WWW_DIR/upload_test_simple.txt" \
        && pass "POST conteudo do arquivo preservado" \
        || fail "POST conteudo do arquivo preservado" "conteudo divergiu"
else
    fail "POST salvou arquivo em www/upload_test_simple.txt" "arquivo nao encontrado"
fi

# 2) sanitizacao de filename: path traversal nao deve escapar de www/
echo "hack content" > "$TMP/hack_src.txt"
assert_status "POST multipart com filename=../../hack.txt" \
    "curl -s -i -X POST $BASE_URL/ -F 'file=@$TMP/hack_src.txt;filename=../../hack.txt'" 201
if [ -f "/hack.txt" ] || [ -f "$SCRIPT_DIR/../../hack.txt" ]; then
    fail "POST path traversal sanitizado" "arquivo escapou para fora de www/"
else
    pass "POST path traversal sanitizado (nada escapou)"
fi

# 3) POST sem multipart -> 400
assert_status "POST sem multipart" \
    "curl -s -i -X POST $BASE_URL/upload_test_raw.txt -d 'raw'" 400

# 4) POST com Content-Type multipart mas body vazio -> 400
assert_status "POST multipart sem boundary util" \
    "curl -s -i -X POST $BASE_URL/ -H 'Content-Type: multipart/form-data; boundary=ABC' -d ''" 400

# 5) Payload acima do client_max_body_size (config: 1M na 8080)
head -c 2000000 </dev/zero | tr '\0' 'A' > "$TMP/big.bin"
assert_status "POST acima de client_max_body_size" \
    "curl -s -i -X POST $BASE_URL/ -F 'file=@$TMP/big.bin'" 413

suite_summary
