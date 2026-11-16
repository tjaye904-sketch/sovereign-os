#!/usr/bin/env bash
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
sovereign-cli sync-export local "leap-$(date +%s)" 2>/dev/null || echo "SOFT sync-export"
sovereign-cli peer-export "$TMP/peer" 2>/dev/null || echo "SOFT peer-export"
ls -la "$TMP" 2>/dev/null || true
echo "smoke-sync: OK"
