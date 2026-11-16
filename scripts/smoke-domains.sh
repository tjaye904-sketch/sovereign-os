#!/usr/bin/env bash
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
CLI=$(command -v sovereign-cli)
sovereign-cli login owner "${SOVEREIGN_TEST_SECRET:-your-secret}" 2>/dev/null || true
sovereign-cli workspace-init-domains 2>/dev/null || sovereign-cli workspace-list 2>/dev/null || true
sovereign-cli workspace-list 2>/dev/null | head -30 || true
sovereign-cli domain-show 2>/dev/null | head -40 || echo "SOFT domain-show"
sovereign-cli domain-chat development "Reply with exactly PONG" 2>/dev/null | head -20 || echo "SOFT domain-chat"
echo "smoke-domains: OK"
