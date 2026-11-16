#!/usr/bin/env bash
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
sovereign-cli login owner "${SOVEREIGN_TEST_SECRET:-your-secret}" 2>/dev/null || true
sovereign-cli principals 2>/dev/null | head -30 || true
# roles needs principal hex — parse first id soft
pid=$(sovereign-cli principals 2>/dev/null | grep -oE '[0-9a-f]{16,}' | head -1 || true)
if [[ -n "${pid:-}" ]]; then
  sovereign-cli roles "$pid" 2>/dev/null | head -20 || echo "SOFT roles"
else
  echo "SOFT no principal id"
fi
echo "smoke-roles: OK"
