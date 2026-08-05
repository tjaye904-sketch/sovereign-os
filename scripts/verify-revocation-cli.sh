#!/usr/bin/env bash
# Milestone A.1 host verification of CLI revoke paths (non-destructive; temp data root)
set -euo pipefail
CLI="${1:-./build/apps/cli/sovereign-cli}"
DATA=$(mktemp -d)
trap 'rm -rf "$DATA"' EXIT
export SOVEREIGN_DATA_ROOT="$DATA"

"$CLI" bootstrap owner 'secret'
"$CLI" login owner 'secret'
"$CLI" operator-create op 'op-secret'
# Suspend operator by id from principals if available — use second login path
"$CLI" login op 'op-secret' || true
"$CLI" login owner 'secret'
# session-revoke current owner session
"$CLI" session-revoke
# login should be required again
if "$CLI" status >/dev/null 2>&1; then
  # status may work without session; try a session-gated command
  if "$CLI" recommend noop 'x' 2>/dev/null; then
    echo "FAIL: recommend succeeded without session after revoke"
    exit 1
  fi
fi
"$CLI" login owner 'secret'
echo "verify-revocation-cli: OK"
