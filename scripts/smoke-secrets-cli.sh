#!/usr/bin/env bash
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
ROOT="${1:-$HOME/src/sovereign-os/sovereign-work}"
CLI="${ROOT}/build/apps/cli/sovereign-cli"
[[ -x "$CLI" ]] || CLI=$(command -v sovereign-cli)
DATA=$(mktemp -d)
export SOVEREIGN_DATA_ROOT="$DATA"
"$CLI" secret-put leap_test 'value-l11'
"$CLI" secret-exists leap_test | grep -q 'exists=1'
"$CLI" secret-list | grep -q leap_test
got=$("$CLI" secret-get leap_test)
[[ "$got" == "value-l11" ]]
"$CLI" secret-delete leap_test
"$CLI" secret-exists leap_test | grep -q 'exists=0'
# audit should not contain the raw secret value
if [[ -f "$DATA/audit/audit.dat" ]]; then
  ! grep -q 'value-l11' "$DATA/audit/audit.dat" || { echo "FAIL secret leaked into audit"; exit 1; }
fi
rm -rf "$DATA"
echo "smoke-secrets-cli: OK"
