#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/src/sovereign-os/sovereign-work}"
OUT="$ROOT/docs/os2.0/HOST_SCORECARD.md"
export PATH="$HOME/.local/bin:$PATH"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
score=0; total=12
check() {
  local name="$1"; shift
  if "$@" >/dev/null 2>&1; then echo "| $name | PASS |"; score=$((score+1)); else echo "| $name | SOFT/FAIL |"; fi
}
{
  echo "# Host scorecard"
  echo
  echo "Generated: $(date -Is)"
  echo "HEAD: $(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null)"
  echo
  echo "| Check | Result |"
  echo "|-------|--------|"
  check "cli" command -v sovereign-cli
  check "daemon_active" systemctl --user is-active sovereign-daemon
  check "ollama" curl -sf http://127.0.0.1:11434/api/tags
  check "cu" curl -sf http://127.0.0.1:8765/health
  check "chat_api" curl -sf http://127.0.0.1:8768/health
  check "hub_status" curl -sf http://127.0.0.1:8766/api/status
  check "ui" test -f "$HOME/.local/share/sovereign/hub-ui/index.html"
  check "ensure" test -x "$HOME/.local/bin/sovereign-ensure-ready"
  check "e2e_script" test -x "$HOME/.local/bin/sovereign-e2e-host"
  check "purposes" test -d "${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}/purposes"
  check "health_timer" systemctl --user is-enabled sovereign-health.timer
  check "on_main" test "$(git -C "$ROOT" branch --show-current)" = main
  echo
  echo "**Score: $score / $total**"
} > "$OUT"
echo "wrote $OUT ($score/$total)"
