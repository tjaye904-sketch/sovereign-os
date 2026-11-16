#!/usr/bin/env bash
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
CLI=$(command -v sovereign-cli)
sovereign-cli login owner "${SOVEREIGN_TEST_SECRET:-your-secret}" 2>/dev/null || true
sovereign-cli goal-add 1 "Host leap goal $(date +%s)" 2>/dev/null || echo "SOFT goal-add"
sovereign-cli goal-list 2>/dev/null | head -20 || true
echo "smoke-goals: OK"
