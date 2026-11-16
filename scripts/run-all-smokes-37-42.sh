#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/src/sovereign-os/sovereign-work}"
export PATH="$HOME/.local/bin:$PATH"
bash "$ROOT/scripts/smoke-roles.sh" || true
bash "$ROOT/scripts/smoke-timing.sh" || true
bash "$ROOT/scripts/smoke-ctl.sh" || true
bash "$ROOT/scripts/write-host-scorecard.sh" "$ROOT"
echo "run-all-smokes-37-42: DONE"
