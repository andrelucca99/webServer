#!/usr/bin/env bash
# Suite: autoindex (listagem de diretorio)
# Cobertura: 200 com HTML listando arquivos quando nao ha index file.

set -u
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SUITE_NAME="AUTOINDEX"
source "$SCRIPT_DIR/lib.sh"

ensure_server_up "$BASE_URL/"
suite_header

WWW_DIR="$(cd "$SCRIPT_DIR/../www" && pwd)"
DIR="$WWW_DIR/autoindex_test"

# fixture: diretorio sem index file, com 2 arquivos conhecidos
mkdir -p "$DIR"
echo "alpha" > "$DIR/alpha.txt"
echo "bravo" > "$DIR/bravo.txt"
trap 'rm -rf "$DIR"' EXIT

# 1) status 200 ao listar
assert_status "GET /autoindex_test/ retorna 200" \
    "curl -s -i $BASE_URL/autoindex_test/" 200

# 2) Content-Type HTML
assert_header "GET /autoindex_test/ Content-Type=text/html" \
    "curl -s -i $BASE_URL/autoindex_test/" "Content-Type" "text/html"

# 3) listagem inclui os arquivos
assert_body "autoindex lista alpha.txt" \
    "curl -s -i $BASE_URL/autoindex_test/" "alpha.txt"
assert_body "autoindex lista bravo.txt" \
    "curl -s -i $BASE_URL/autoindex_test/" "bravo.txt"

# 4) titulo da pagina inclui o path
assert_body "autoindex titulo inclui path" \
    "curl -s -i $BASE_URL/autoindex_test/" "Index of"

# 5) na presenca de index file, NAO faz autoindex (serve o index)
assert_status "GET / serve index.html (sem autoindex)" \
    "curl -s -i $BASE_URL/" 200
out=$(curl -s "$BASE_URL/" 2>/dev/null)
if echo "$out" | grep -q "Index of"; then
    fail "GET / nao deve fazer autoindex (existe index.html)" "encontrou 'Index of' no body"
else
    pass "GET / serve index.html (autoindex nao executou)"
fi

# 6) arquivo dentro do dir do autoindex continua acessivel diretamente
assert_status "GET /autoindex_test/alpha.txt direto" \
    "curl -s -i $BASE_URL/autoindex_test/alpha.txt" 200
assert_header "GET alpha.txt Content-Type=text/plain" \
    "curl -s -i $BASE_URL/autoindex_test/alpha.txt" "Content-Type" "text/plain"

suite_summary
