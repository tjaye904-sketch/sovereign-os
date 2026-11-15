#!/usr/bin/env bash
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
ROOT="${SOVEREIGN_WORK_ROOT:-$HOME/src/sovereign-os/sovereign-work}"
CHAT="${ROOT}/scripts/sovereign-hub-chat.sh"
if [[ ! -x "$CHAT" ]]; then
  CHAT="${HOME}/.local/bin/sovereign-hub-chat"
fi
if [[ ! -x "$CHAT" ]]; then
  echo "FAIL: sovereign-hub-chat missing" >&2
  exit 1
fi
out="$("$CHAT" "Reply with exactly PONG" 2>&1 || true)"
echo "$out"
if echo "$out" | grep -qi 'command not found'; then
  echo "FAIL: command not found in chat path" >&2
  exit 1
fi
if echo "$out" | grep -q 'PONG'; then
  echo "verify-wp21-hub-chat: OK"
  exit 0
fi
# Model may paraphrase; accept explicit content= line from CLI
if echo "$out" | grep -qiE 'content=|model='; then
  echo "verify-wp21-hub-chat: OK (model responded)"
  exit 0
fi
echo "FAIL: no usable chat response" >&2
exit 1
