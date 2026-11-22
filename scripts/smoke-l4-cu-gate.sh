#!/usr/bin/env bash
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
export SOVEREIGN_DATA_ROOT="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}"
CLI="${SOVEREIGN_CLI:-$HOME/src/sovereign-os/sovereign-work/build/apps/cli/sovereign-cli}"
[[ -x "$CLI" ]] || CLI=$(command -v sovereign-cli)

"$CLI" capability-seed >/dev/null 2>&1 || true
"$CLI" capability-enable host.computer_use >/dev/null 2>&1 || true
"$CLI" capability-enable host.purpose >/dev/null 2>&1 || true

echo "=== enable path ==="
"$CLI" capability-gate-check browser_open || true

echo "=== disable host.computer_use ==="
"$CLI" capability-disable host.computer_use
"$CLI" capability-gate-check browser_open || true

# CU server if up
if curl -sf http://127.0.0.1:8765/health >/dev/null 2>&1; then
  echo "=== CU purpose_run should deny when computer_use disabled ==="
  curl -s -X POST http://127.0.0.1:8765/v1/action \
    -H 'Content-Type: application/json' \
    -d '{"action":"browser_open","args":{"url":"https://docs.python.org/3/"}}' || true
  echo
fi

"$CLI" capability-enable host.computer_use
echo "smoke-l4-cu-gate: OK (re-enabled host.computer_use)"
