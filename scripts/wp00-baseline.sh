#!/usr/bin/env bash
# WP-00: capture baseline into docs/os2.0/BASELINE_<date>.md (or stdout path)
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
ROOT="${1:-${HOME}/src/sovereign-os/sovereign-work}"
DATE="$(date -u +%Y%m%dT%H%M%SZ)"
OUT="${ROOT}/docs/os2.0/BASELINE_${DATE}.md"
mkdir -p "${ROOT}/docs/os2.0"

{
  echo "# Sovereign baseline ${DATE}"
  echo
  echo "## Host"
  echo '```'
  uname -a || true
  echo "user=$USER"
  echo '```'
  echo
  echo "## Git"
  echo '```'
  if [[ -d "${ROOT}/.git" ]]; then
    git -C "$ROOT" rev-parse HEAD 2>/dev/null || true
    git -C "$ROOT" log --oneline -5 2>/dev/null || true
  else
    echo "no git at $ROOT"
  fi
  echo '```'
  echo
  echo "## systemd --user"
  echo '```'
  systemctl --user is-active sovereign-daemon sovereign-hub sovereign-sys sovereign-computer-use 2>&1 || true
  echo '```'
  echo
  echo "## HTTP"
  echo '```'
  curl -sS --connect-timeout 2 http://127.0.0.1:8765/health 2>&1 || echo "8765 down"
  echo
  curl -sS --connect-timeout 2 http://127.0.0.1:8766/api/status 2>&1 || echo "8766 down"
  echo
  curl -sS --connect-timeout 2 http://127.0.0.1:8767/health 2>&1 || echo "8767 down"
  echo
  echo '```'
  echo
  echo "## sovereign-cli status"
  echo '```'
  sovereign-cli status 2>&1 || echo "cli missing"
  echo '```'
  echo
  echo "## Env (names only)"
  echo '```'
  if [[ -f "${HOME}/.config/sovereign/env" ]]; then
    grep -E '^[A-Z_]+=' "${HOME}/.config/sovereign/env" | cut -d= -f1 || true
  else
    echo "no env file"
  fi
  echo '```'
  echo
  echo "## Known gaps (pathway)"
  echo "- Curiosity engine not implemented"
  echo "- Observe-after-act not automatic"
  echo "- CU requires graphical session env"
  echo "- Chat router incomplete"
  echo
  echo "Pathway: docs/os2.0/COMPLETION_PATHWAY.md"
} > "$OUT"

echo "Wrote $OUT"
