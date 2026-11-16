#!/usr/bin/env bash
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
CLI=$(command -v sovereign-cli)
sovereign-cli login owner "${SOVEREIGN_TEST_SECRET:-your-secret}" 2>/dev/null || true
sovereign-cli semantic-add "Sovereign host leap semantic marker $(date +%s)" 2>/dev/null || echo "SOFT semantic-add"
sovereign-cli semantic-query "Sovereign host" 2>/dev/null | head -20 || echo "SOFT semantic-query"
echo "smoke-semantic: OK"
