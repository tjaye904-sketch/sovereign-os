#!/usr/bin/env bash
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
fail=0
check() {
  local name="$1"; shift
  if "$@"; then echo "PASS $name"; else echo "FAIL $name"; fail=1; fi
}
check cli test -x "$HOME/.local/bin/sovereign-cli" -o -x "$HOME/src/sovereign-os/sovereign-work/build/apps/cli/sovereign-cli"
check daemon systemctl --user is-active sovereign-daemon
check ollama curl -sf http://127.0.0.1:11434/api/tags
check chat_api curl -sf http://127.0.0.1:8768/health
check hub_chat test -x "$HOME/.local/bin/sovereign-hub-chat"
check ui test -f "$HOME/.local/share/sovereign/hub-ui/index.html"
check ensure test -x "$HOME/.local/bin/sovereign-ensure-ready"
# soft
curl -sf http://127.0.0.1:8765/health >/dev/null && echo "PASS cu" || echo "SOFT cu down"
curl -sf http://127.0.0.1:8766/api/status >/dev/null && echo "PASS hub8766" || echo "SOFT hub8766 down"
if [[ -x "$HOME/.local/bin/sovereign-hub-chat" ]]; then
  out=$(sovereign-hub-chat "Reply with exactly PONG" 2>&1 || true)
  echo "$out" | grep -q PONG && echo "PASS chat_pong" || echo "SOFT chat_pong ($out)" | head -c 200
fi
exit "$fail"
