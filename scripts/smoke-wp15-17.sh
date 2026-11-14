#!/usr/bin/env bash
# WP-17: full host smoke + matrix marker
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
ROOT="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}"
EVID="$ROOT/evidence"
mkdir -p "$EVID"
STAMP=$(date -u +%Y%m%dT%H%M%SZ)
LOG="$EVID/wp17-smoke-$STAMP.log"
exec > >(tee -a "$LOG") 2>&1

echo "=== WP-17 smoke $STAMP ==="
fail=0
check() {
  local name="$1"; shift
  if "$@"; then echo "PASS $name"; else echo "FAIL $name"; fail=$((fail+1)); fi
}

check units systemctl --user is-active sovereign-daemon
check hub curl -sf http://127.0.0.1:8766/api/status >/dev/null
check cu curl -sf http://127.0.0.1:8765/health >/dev/null
check sys curl -sf http://127.0.0.1:8767/health >/dev/null
check shot bash -c 'sovereign-cu desktop_screenshot --args "{\"name\":\"wp17.png\"}" | grep -q "\"ok\": true"'
check tools bash -c 'sovereign-tools --json "open system monitor" | grep -q "\"ok\": true"'

echo "=== WP-17 done failures=$fail log=$LOG ==="
[[ "$fail" -eq 0 ]]
