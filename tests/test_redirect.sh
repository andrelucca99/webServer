#!/usr/bin/env bash
# Suite: redirect (return 301)
# Cobertura: status 301, header Location, longest-prefix match aplica redirect.

set -u
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SUITE_NAME="REDIRECT"
source "$SCRIPT_DIR/lib.sh"

ensure_server_up "$BASE_URL/"
suite_header

# Config: location /old { return 301 /; }
assert_status "GET /old retorna 301" \
    "curl -s -i $BASE_URL/old" 301
assert_header "GET /old tem Location: /" \
    "curl -s -i $BASE_URL/old" "Location" "/"

# longest-prefix: /old/foo tambem casa com /old
assert_status "GET /old/foo retorna 301 (longest-prefix)" \
    "curl -s -i $BASE_URL/old/foo" 301
assert_header "GET /old/foo tem Location: /" \
    "curl -s -i $BASE_URL/old/foo" "Location" "/"

# curl segue redirect e termina em 200
assert_status "GET /old com -L termina em 200" \
    "curl -s -i -L $BASE_URL/old" 200

# rotas sem return nao mandam Location
out=$(curl -s -i "$BASE_URL/" 2>/dev/null)
if echo "$out" | grep -qi "^Location:"; then
    fail "GET / nao deve ter header Location" "Location apareceu numa rota sem return"
else
    pass "GET / nao tem Location (rota sem redirect)"
fi

suite_summary
