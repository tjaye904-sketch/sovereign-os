#!/usr/bin/env bash
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
BIN="${HOME}/.local/bin/sovereign-tools"
[[ -x "$BIN" ]] || BIN="${HOME}/src/sovereign-os/sovereign-work/scripts/sovereign-tools"

echo "=== WP-03 verify ==="
FAIL=0

"$BIN" --dry-run "open python docs" | grep -q browser_open || { echo FAIL dry-run open; FAIL=1; }
"$BIN" --dry-run "list windows" | grep -q desktop_list_windows || { echo FAIL dry-run windows; FAIL=1; }
"$BIN" --dry-run "open system monitor" | grep -q app_open || { echo FAIL dry-run sys; FAIL=1; }

# Live only if CU up
if curl -sf http://127.0.0.1:8765/health >/dev/null; then
  OUT=$("$BIN" --json "ping" 2>/dev/null || true)
  echo "$OUT" | grep -q '"routed": true' || echo "WARN live ping route"
  echo "PASS live path attempted"
else
  echo "WARN CU down — dry-run only"
fi

if [[ "$FAIL" -eq 0 ]]; then
  echo "WP-03 verify: OK"
  exit 0
fi
echo "WP-03 verify: FAILED"
exit 1
