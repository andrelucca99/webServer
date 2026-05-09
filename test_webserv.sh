#!/usr/bin/env bash
# Wrapper de compatibilidade: a suite de integracao foi modularizada em tests/.
# Veja tests/README.md para detalhes.
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
exec bash "$SCRIPT_DIR/tests/run_all.sh" "$@"
