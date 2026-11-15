#!/usr/bin/env bash
# Mission Control / terminal bridge: always invoke a real Sovereign binary.
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
if [[ -f "${HOME}/.config/sovereign/env.bash" ]]; then
  # shellcheck disable=SC1091
  source "${HOME}/.config/sovereign/env.bash"
elif [[ -f "${HOME}/.config/sovereign/env" ]]; then
  set -a
  # shellcheck disable=SC1091
  source "${HOME}/.config/sovereign/env"
  set +a
fi
export OLLAMA_HOST="${OLLAMA_HOST:-http://127.0.0.1:11434}"
export SOVEREIGN_OLLAMA_MODEL="${SOVEREIGN_OLLAMA_MODEL:-granite4.1:3b}"
export SOVEREIGN_DATA_ROOT="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}"

CLI=""
for c in \
  "${HOME}/.local/bin/sovereign-cli" \
  "${HOME}/src/sovereign-os/sovereign-work/build/apps/cli/sovereign-cli" \
  "$(command -v sovereign-cli 2>/dev/null || true)"; do
  if [[ -n "$c" && -x "$c" ]]; then
    CLI="$c"
    break
  fi
done
if [[ -z "$CLI" ]]; then
  echo "error: sovereign-cli not found (install/build host binaries)" >&2
  exit 127
fi

MODE="${SOVEREIGN_HUB_CHAT_MODE:-chat}"  # chat | chat-tools
PROMPT="${*:-}"
if [[ -z "$PROMPT" ]]; then
  echo "usage: sovereign-hub-chat.sh <prompt...>" >&2
  exit 2
fi

# Best-effort session: do not fail chat if already logged in elsewhere
if ! "$CLI" status >/dev/null 2>&1; then
  true
fi

if [[ "$MODE" == "chat-tools" ]] && "$CLI" 2>&1 | grep -q 'chat-tools'; then
  exec "$CLI" chat-tools "$PROMPT"
fi
exec "$CLI" chat "$PROMPT"
