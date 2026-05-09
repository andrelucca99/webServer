#!/usr/bin/env bash
# Suite: DELETE
# Cobertura: 204 ao remover, 404 em arquivo inexistente, idempotencia.

set -u
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SUITE_NAME="DELETE"
source "$SCRIPT_DIR/lib.sh"

ensure_server_up "$BASE_URL/"
suite_header

WWW_DIR="$(cd "$SCRIPT_DIR/../www" && pwd)"
trap 'rm -f "$WWW_DIR"/delete_test_*.txt' EXIT

# fixture: cria arquivo que sera deletado
echo "to delete" > "$WWW_DIR/delete_test_a.txt"

# 1) DELETE em arquivo existente -> 204
assert_status "DELETE /delete_test_a.txt (existente)" \
    "curl -s -i -X DELETE $BASE_URL/delete_test_a.txt" 204
if [ ! -f "$WWW_DIR/delete_test_a.txt" ]; then
    pass "DELETE removeu arquivo do disco"
else
    fail "DELETE removeu arquivo do disco" "arquivo ainda existe"
fi

# 2) DELETE no mesmo arquivo (agora ausente) -> 404
assert_status "DELETE /delete_test_a.txt (segunda vez)" \
    "curl -s -i -X DELETE $BASE_URL/delete_test_a.txt" 404

# 3) DELETE em arquivo nunca criado -> 404
assert_status "DELETE /delete_test_nope.txt" \
    "curl -s -i -X DELETE $BASE_URL/delete_test_nope.txt" 404

# 4) DELETE deve ser bloqueado no server somente-GET (porta 8081) -> 405
assert_status "DELETE /index.html na 8081 (somente GET)" \
    "curl -s -i -X DELETE $BASE_URL_RO/index.html" 405

suite_summary
