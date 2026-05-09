#!/usr/bin/env bash
# Helper interno: sobe webserv_test em background, roda run_all.sh e mata o servidor.
# Usado para rodar a suite localmente em WSL.
set -u
cd "$(dirname "$0")/.."

pkill -f ./webserv_test 2>/dev/null
sleep 0.3

./webserv_test > /tmp/webserv.log 2>&1 &
PID=$!
sleep 0.5

if ! kill -0 "$PID" 2>/dev/null; then
    echo "[!] webserv_test nao subiu. Log:"
    cat /tmp/webserv.log
    exit 1
fi

echo "[*] webserv_test PID=$PID"
trap 'kill "$PID" 2>/dev/null; wait "$PID" 2>/dev/null' EXIT INT TERM

bash tests/run_all.sh
rc=$?

echo
echo "--- server log (tail 20) ---"
tail -20 /tmp/webserv.log
exit $rc
