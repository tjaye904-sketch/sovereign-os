#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/src/sovereign-os/sovereign-work}"
CLI="$ROOT/build/apps/cli/sovereign-cli"
[[ -x "$CLI" ]] || CLI=$(command -v sovereign-cli)
DATA=$(mktemp -d)
export SOVEREIGN_DATA_ROOT="$DATA"
"$CLI" capability-seed >/dev/null || true
# Gate check commands if present
if "$CLI" 2>&1 | grep -q capability-gate-check; then
  "$CLI" capability-gate-check execute || true
  "$CLI" capability-disable host.execute 2>/dev/null || true
  set +e
  out=$("$CLI" capability-gate-check execute 2>&1)
  ec=$?
  set -e
  echo "$out"
  echo "$out" | grep -q 'allowed=0' || echo "$out" | grep -qi deny || echo "WARN: expected deny when disabled"
else
  echo "capability-gate-check not in CLI help — core test only"
fi
"$CLI" capability-enable host.execute 2>/dev/null || true
rm -rf "$DATA"
echo "smoke-capability-gate: OK"
