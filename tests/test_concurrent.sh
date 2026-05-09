#!/usr/bin/env bash
# Suite: conexoes concorrentes
# Cobertura: o servidor aguenta varias requisicoes em paralelo, nas duas portas,
# sem trancar nem retornar erros.

set -u
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SUITE_NAME="CONCURRENT"
source "$SCRIPT_DIR/lib.sh"

ensure_server_up "$BASE_URL/"
ensure_server_up "$BASE_URL_RO/"
suite_header

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

# 1) 50 GETs paralelos na 8080 -> todos 200
N=50
for i in $(seq 1 $N); do
    (curl -s -o /dev/null -w "%{http_code}\n" "$BASE_URL/" > "$TMP/out_$i") &
done
wait

ok=0
for i in $(seq 1 $N); do
    code=$(cat "$TMP/out_$i" 2>/dev/null)
    [ "$code" = "200" ] && ok=$((ok + 1))
done
if [ $ok -eq $N ]; then
    pass "$N GETs paralelos -> todos 200"
else
    fail "$N GETs paralelos" "$ok/$N retornaram 200"
fi

# 2) Mistura de GET/POST/DELETE em paralelo
WWW_DIR="$(cd "$SCRIPT_DIR/../www" && pwd)"
echo "x" > "$WWW_DIR/concurrent_a.txt"
echo "y" > "$WWW_DIR/concurrent_b.txt"
trap 'rm -rf "$TMP"; rm -f "$WWW_DIR"/concurrent_*.txt' EXIT

(curl -s -o /dev/null -w "%{http_code}" "$BASE_URL/" > "$TMP/m_get") &
(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$BASE_URL/concurrent_a.txt" > "$TMP/m_del") &
(echo "p" > "$TMP/up.txt"; curl -s -o /dev/null -w "%{http_code}" -X POST "$BASE_URL/" -F "file=@$TMP/up.txt" > "$TMP/m_post") &
(curl -s -o /dev/null -w "%{http_code}" "$BASE_URL_RO/" > "$TMP/m_ro") &
wait

[ "$(cat "$TMP/m_get")" = "200" ] && pass "concorrente: GET 8080 -> 200" || fail "concorrente: GET 8080" "veio $(cat "$TMP/m_get")"
[ "$(cat "$TMP/m_del")" = "204" ] && pass "concorrente: DELETE 8080 -> 204" || fail "concorrente: DELETE 8080" "veio $(cat "$TMP/m_del")"
[ "$(cat "$TMP/m_post")" = "201" ] && pass "concorrente: POST 8080 -> 201" || fail "concorrente: POST 8080" "veio $(cat "$TMP/m_post")"
[ "$(cat "$TMP/m_ro")"  = "200" ] && pass "concorrente: GET 8081 -> 200" || fail "concorrente: GET 8081" "veio $(cat "$TMP/m_ro")"

# 3) Servidor nao deve travar apos uma conexao que abre e nao envia nada
( exec 3<>/dev/tcp/127.0.0.1/8080; sleep 0.3; exec 3<&-; exec 3>&- ) &
SLOW_PID=$!
sleep 0.05
code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 2 "$BASE_URL/")
wait $SLOW_PID 2>/dev/null
if [ "$code" = "200" ]; then
    pass "servidor nao trava com cliente ocioso paralelo"
else
    fail "servidor nao trava com cliente ocioso paralelo" "GET concorrente devolveu $code"
fi

suite_summary
