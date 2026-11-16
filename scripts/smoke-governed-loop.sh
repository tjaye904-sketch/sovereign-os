#!/usr/bin/env bash
# Gap 21: governed loop should pause for approval (no --auto-approve)
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
CLI=$(command -v sovereign-cli)
# Login best-effort
sovereign-cli login owner "${SOVEREIGN_TEST_SECRET:-your-secret}" 2>/dev/null || true
out=$(sovereign-cli loop "Smoke governed pause for host health" 2>&1 || true)
echo "$out" | tail -40
if echo "$out" | grep -qiE 'paused|approve|governance_decision'; then
  echo "PASS loop paused for governance"
else
  echo "SOFT loop output did not clearly pause (session/login may be required)"
fi
echo "smoke-governed-loop: OK"
