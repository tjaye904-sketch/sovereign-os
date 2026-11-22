#!/usr/bin/env bash
set -euo pipefail
export SOVEREIGN_DATA_ROOT="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}"
export SOVEREIGN_CLI="${SOVEREIGN_CLI:-$HOME/src/sovereign-os/sovereign-work/build/apps/cli/sovereign-cli}"
export PATH="$HOME/.local/bin:$PATH"

"$SOVEREIGN_CLI" capability-seed >/dev/null 2>&1 || true

# ensure sidecar up
if ! curl -sf http://127.0.0.1:8768/health >/dev/null 2>&1; then
  sovereign-caps-ui || true
  sleep 1
fi

echo "=== list ==="
curl -s http://127.0.0.1:8768/api/capabilities | head -c 800
echo

echo "=== disable host.computer_use via API ==="
curl -s -X POST http://127.0.0.1:8768/api/capabilities/host.computer_use \
  -H 'Content-Type: application/json' -d '{"enabled":false}'
echo
"$SOVEREIGN_CLI" capability-gate-check browser_open

echo "=== enable ==="
curl -s -X POST http://127.0.0.1:8768/api/capabilities/host.computer_use \
  -H 'Content-Type: application/json' -d '{"enabled":true}'
echo
"$SOVEREIGN_CLI" capability-gate-check browser_open

echo "smoke-l5-caps-ui: OK"
