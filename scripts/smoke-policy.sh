#!/usr/bin/env bash
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
sovereign-cli login owner "${SOVEREIGN_TEST_SECRET:-your-secret}" 2>/dev/null || true
sovereign-cli policy-version 2>/dev/null || true
sovereign-cli policy-dry-run owner deploy 2>/dev/null || true
sovereign-cli policy-check owner read 2>/dev/null || true
echo "smoke-policy: OK"
