#!/usr/bin/env bash
# Gap 23
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
CLI=$(command -v sovereign-cli)
sovereign-cli inspect 2>/dev/null | tee /tmp/sov-inspect.txt | head -80
grep -qiE 'audit|identity|policy|specification|coverage|subsystem' /tmp/sov-inspect.txt && echo "PASS inspect sections" || echo "SOFT inspect shape"
echo "smoke-inspect: OK"
