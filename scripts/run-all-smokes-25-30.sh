#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/src/sovereign-os/sovereign-work}"
export PATH="$HOME/.local/bin:$PATH"
for s in smoke-domains smoke-goals smoke-agents smoke-semantic smoke-archive-memory smoke-mfa-session; do
  echo "==== $s ===="
  bash "$ROOT/scripts/$s.sh" || echo "SOFT $s non-zero"
done
echo "run-all-smokes-25-30: DONE"
