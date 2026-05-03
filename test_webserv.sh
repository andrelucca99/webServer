#!/bin/bash

BASE_URL="http://127.0.0.1:8080"

GREEN="\033[0;32m"
RED="\033[0;31m"
NC="\033[0m"

pass() { echo -e "${GREEN}✔ $1${NC}"; }
fail() { echo -e "${RED}✘ $1${NC}"; }

check_status() {
    NAME="$1"
    CMD="$2"
    EXPECTED="$3"

    STATUS=$(eval "$CMD" | head -n 1 | cut -d' ' -f2)

    if [ "$STATUS" == "$EXPECTED" ]; then
        pass "$NAME ($STATUS)"
    else
        fail "$NAME (esperado $EXPECTED, veio $STATUS)"
    fi
}

echo "=============================="
echo " TESTES WEBSERV"
echo "=============================="

# ======================
# 200 OK
# ======================
check_status "GET index" \
"curl -s -i $BASE_URL/" \
"200"

# ======================
# 404 NOT FOUND
# ======================
check_status "GET arquivo inexistente" \
"curl -s -i $BASE_URL/naoexiste.html" \
"404"

# ======================
# 400 BAD REQUEST
# ======================
check_status "POST sem multipart" \
"curl -s -i -X POST $BASE_URL/test.txt -d 'hack'" \
"400"

check_status "POST path traversal" \
"curl -s -i -X POST $BASE_URL/../../hack.txt -d 'hack'" \
"400"

check_status "POST sem Content-Length válido" \
"printf 'POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: abc\r\n\r\nbody' | nc 127.0.0.1 8080" \
"400"

# ======================
# 405 METHOD NOT ALLOWED
# ======================
check_status "PUT não permitido" \
"curl -s -i -X PUT $BASE_URL/" \
"405"

# ======================
# 505 HTTP VERSION
# ======================
check_status "HTTP versão inválida" \
"printf 'GET / HTTP/2.0\r\nHost: localhost\r\n\r\n' | nc 127.0.0.1 8080" \
"505"

# ======================
# 413 PAYLOAD TOO LARGE
# ======================
head -c 2000000 </dev/zero | tr '\0' 'A' > big.txt

check_status "Payload muito grande" \
"curl -s -i -X POST $BASE_URL/upload --data-binary @big.txt" \
"413"

# ======================
# DELETE
# ======================
echo "test" > ./www/delete_me.txt

check_status "DELETE existente" \
"curl -s -i -X DELETE $BASE_URL/delete_me.txt" \
"204"

check_status "DELETE inexistente" \
"curl -s -i -X DELETE $BASE_URL/delete_me.txt" \
"404"

# ======================
# MULTIPART
# ======================
echo "file content" > test.txt

check_status "Multipart simples" \
"curl -s -i -X POST $BASE_URL/ -F 'file=@test.txt'" \
"201"

check_status "Multipart com path traversal" \
"curl -s -i -X POST $BASE_URL/ -F 'file=@test.txt;filename=../../hack.txt'" \
"201"

# ======================
# AUTOINDEX
# ======================
check_status "GET diretório (autoindex/index)" \
"curl -s -i $BASE_URL/" \
"200"

# ======================
# SEGURANÇA
# ======================
check_status "Path traversal encoded" \
"curl -s -i $BASE_URL/%2e%2e/%2e%2e/etc/passwd" \
"403"

echo "=============================="
echo " FIM DOS TESTES"
echo "=============================="