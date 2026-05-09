#!/usr/bin/env bash
# Roda todas as suites tests/test_*.sh e agrega o resultado.

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

TOTAL_PASS=0
TOTAL_FAIL=0
FAILED_SUITES=()

for suite in "$SCRIPT_DIR"/test_*.sh; do
    [ -e "$suite" ] || continue
    bash "$suite"
    rc=$?
    if [ $rc -ne 0 ]; then
        FAILED_SUITES+=("$(basename "$suite")")
    fi
    echo
done

echo "=============================="
if [ ${#FAILED_SUITES[@]} -eq 0 ]; then
    echo "  Todas as suites passaram."
    exit 0
else
    echo "  Suites com falha:"
    for s in "${FAILED_SUITES[@]}"; do
        echo "   - $s"
    done
    exit 1
fi
