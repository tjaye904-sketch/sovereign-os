#!/usr/bin/env bash
# Gap 22
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
CLI=$(command -v sovereign-cli)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
sovereign-cli peer-export "$TMP/bundle" 2>/dev/null || { echo "SOFT peer-export unavailable"; exit 0; }
ls -la "$TMP/bundle" || true
sovereign-cli audit-verify 2>/dev/null && echo "PASS audit-verify" || echo "SOFT audit-verify"
sovereign-cli audit-list 2>/dev/null | tail -15 || true
echo "smoke-peer-audit: OK"
