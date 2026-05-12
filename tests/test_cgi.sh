#!/usr/bin/env bash
# Suite: CGI
# Cobertura: GET com query string, POST com body via stdin, header custom do CGI,
# redirect (Status: 302 + Location), script inexistente (5xx) e timeout (504).

set -u
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SUITE_NAME="CGI"
source "$SCRIPT_DIR/lib.sh"

ensure_server_up "$BASE_URL/"
suite_header

# GET com query string
assert_status "GET /cgi-bin/hello.py?name=alice" \
    "curl -s -i '$BASE_URL/cgi-bin/hello.py?name=alice'" 200
assert_body "GET hello.py ecoa query" \
    "curl -s -i '$BASE_URL/cgi-bin/hello.py?name=alice'" "query=name=alice"
assert_body "GET hello.py ecoa method=GET" \
    "curl -s -i '$BASE_URL/cgi-bin/hello.py?name=alice'" "method=GET"

# Header custom do CGI eh preservado
assert_header "GET hello.py X-CGI-Echo" \
    "curl -s -i '$BASE_URL/cgi-bin/hello.py'" "X-CGI-Echo" "hello"

# Content-Type default vem do CGI
assert_header "GET hello.py Content-Type=text/html" \
    "curl -s -i '$BASE_URL/cgi-bin/hello.py'" "Content-Type" "text/html"

# POST com body via stdin
assert_status "POST /cgi-bin/hello.py" \
    "curl -s -i -X POST -H 'Content-Type: text/plain' --data 'hello-from-stdin' '$BASE_URL/cgi-bin/hello.py'" 200
assert_body "POST hello.py ecoa stdin" \
    "curl -s -i -X POST -H 'Content-Type: text/plain' --data 'hello-from-stdin' '$BASE_URL/cgi-bin/hello.py'" "body=hello-from-stdin"
assert_body "POST hello.py method=POST" \
    "curl -s -i -X POST -H 'Content-Type: text/plain' --data 'hello-from-stdin' '$BASE_URL/cgi-bin/hello.py'" "method=POST"

# CGI emitindo Status: 302 + Location
assert_status "GET /cgi-bin/redirect.py (Status: 302)" \
    "curl -s -i '$BASE_URL/cgi-bin/redirect.py'" 302
assert_header "GET /cgi-bin/redirect.py Location: /" \
    "curl -s -i '$BASE_URL/cgi-bin/redirect.py'" "Location" "/"

# Script inexistente -> exec falha -> 502
assert_status "GET /cgi-bin/missing.py -> 502" \
    "curl -s -i '$BASE_URL/cgi-bin/missing.py'" 502

# Script lento -> timeout do servidor -> 504
assert_status "GET /cgi-bin/slow.py -> 504" \
    "curl -s -i --max-time 10 '$BASE_URL/cgi-bin/slow.py'" 504

suite_summary
