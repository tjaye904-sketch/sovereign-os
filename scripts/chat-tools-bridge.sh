#!/usr/bin/env bash
# WP-13: sovereign-cli chat-tools → sovereign-tools when NL matches a tool route
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
PROMPT="${*:-}"
if [[ -z "$PROMPT" ]]; then
  echo "usage: sovereign-chat-tools <prompt...>"
  exit 2
fi

# Prefer tools router
if command -v sovereign-tools >/dev/null; then
  OUT=$(sovereign-tools --json "$PROMPT" 2>/dev/null || true)
  if echo "$OUT" | grep -q '"routed": true'; then
    echo "$OUT"
    # human summary
    if echo "$OUT" | grep -q '"ok": true'; then
      exit 0
    fi
    # if routed but denied/failed, still exit non-zero but showed JSON
    exit 1
  fi
fi

# Fall back to model chat
exec sovereign-cli chat "$PROMPT"
