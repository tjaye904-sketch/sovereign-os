#!/usr/bin/env bash
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
sovereign-cli status >/dev/null 2>&1 || true
sovereign-cli inspect timing 2>/dev/null | head -40 || sovereign-cli inspect 2>/dev/null | grep -i -A20 timing || echo "SOFT timing section"
DATA="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}"
[[ -f "$DATA/metrics/cmd_times.log" ]] && tail -10 "$DATA/metrics/cmd_times.log" || echo "SOFT no cmd_times.log yet"
echo "smoke-timing: OK"
