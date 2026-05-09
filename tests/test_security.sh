#!/usr/bin/env bash
# Suite: seguranca (path traversal)
# Cobertura: bloqueio de '..' literal e URL-encoded em GET/DELETE/POST.

set -u
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SUITE_NAME="SECURITY"
source "$SCRIPT_DIR/lib.sh"

ensure_server_up "$BASE_URL/"
suite_header

# 1) .. literal no path
assert_status "GET /../etc/passwd -> 403" \
    "curl -s -i $BASE_URL/../etc/passwd" 403

# 2) .. URL-encoded (%2e%2e)
assert_status "GET /%2e%2e/etc/passwd -> 403" \
    "curl -s -i $BASE_URL/%2e%2e/etc/passwd" 403

# 3) .. duplo URL-encoded misturado
assert_status "GET com %2e%2e%2f -> 403" \
    "curl -s -i $BASE_URL/%2e%2e%2f%2e%2e%2fetc%2fpasswd" 403

# 4) DELETE com path traversal
assert_status "DELETE /../hack -> 403" \
    "curl -s -i -X DELETE $BASE_URL/../hack" 403

# 5) garantia: o body de erro nao expoe conteudo de fora de www/
out=$(curl -s "$BASE_URL/%2e%2e/etc/passwd" 2>/dev/null)
if echo "$out" | grep -q "root:x:"; then
    fail "GET path traversal nao deve vazar /etc/passwd" "encontrou conteudo do passwd no body"
else
    pass "GET path traversal nao vaza /etc/passwd"
fi

suite_summary
