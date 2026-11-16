#!/usr/bin/env bash
# Item 6: unified cognitive surface via existing CLI where present, else document gaps
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
CLI=$(command -v sovereign-cli || true)
[[ -x "$CLI" ]] || CLI="$HOME/src/sovereign-os/sovereign-work/build/apps/cli/sovereign-cli"
[[ -x "$CLI" ]] || { echo "sovereign-cli missing"; exit 127; }
cmd="${1:-help}"
shift || true
case "$cmd" in
  help|-h|--help)
    cat << H
sovereign-cognitive-cli <cmd>
  plan <goal...>       → sovereign-cli plan
  chat <prompt...>     → sovereign-cli chat / hub-chat
  loop <goal...>       → sovereign-cli loop
  reflect              → memory-add reflection tag if no reflect cmd
  status               → sovereign-cli status
  reason|curiosity|orchestrate  → try native CLI subcommands if built
H
    ;;
  plan) exec "$CLI" plan "$@" ;;
  chat)
    if command -v sovereign-hub-chat >/dev/null; then exec sovereign-hub-chat "$*"; fi
    exec "$CLI" chat "$*"
    ;;
  loop) exec "$CLI" loop "$@" ;;
  status) exec "$CLI" status ;;
  reflect)
    "$CLI" memory-add reflection "$*" 2>/dev/null || "$CLI" memory-add note "reflection: $*"
    ;;
  curiosity|reason|orchestrate)
    if "$CLI" 2>&1 | grep -q "$cmd"; then
      exec "$CLI" "$cmd" "$@"
    else
      echo "native '$cmd' not in this CLI build — use plan/loop; engines exist in C++ library"
      exit 0
    fi
    ;;
  *) echo "unknown $cmd"; exit 2 ;;
esac
