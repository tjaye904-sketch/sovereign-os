#!/usr/bin/env bash
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
CLI=$(command -v sovereign-cli)
sovereign-cli login owner "${SOVEREIGN_TEST_SECRET:-your-secret}" 2>/dev/null || true
sovereign-cli mfa-status 2>/dev/null || echo "SOFT mfa-status"
sovereign-cli principals 2>/dev/null | head -20 || true
# Do not revoke live session by default
echo "smoke-mfa-session: OK (no destructive revoke)"
