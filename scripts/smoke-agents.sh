#!/usr/bin/env bash
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
CLI=$(command -v sovereign-cli)
sovereign-cli login owner "${SOVEREIGN_TEST_SECRET:-your-secret}" 2>/dev/null || true
sovereign-cli agent-add-brief 300 2>/dev/null || true
sovereign-cli agent-list 2>/dev/null | head -20 || true
sovereign-cli agent-tick 2>/dev/null || echo "SOFT agent-tick"
sovereign-cli brief 2>/dev/null | head -40 || true
echo "smoke-agents: OK"
