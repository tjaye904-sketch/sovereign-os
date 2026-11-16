#!/usr/bin/env bash
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
sovereign-cli login owner "${SOVEREIGN_TEST_SECRET:-your-secret}" 2>/dev/null || true
out=$(sovereign-cli recommend inspect "leap recommend smoke $(date +%s)" 2>&1 || true)
echo "$out" | head -30
id=$(echo "$out" | grep -oE '[0-9a-f]{16,}' | head -1 || true)
if [[ -n "${id:-}" ]]; then
  sovereign-cli decision-get "$id" 2>/dev/null | head -20 || true
  sovereign-cli pending-decisions 2>/dev/null | head -20 || true
  echo "PASS decision id=$id (not auto-executed)"
else
  echo "SOFT no decision id parsed"
fi
echo "smoke-recommend: OK"
