#!/usr/bin/env bash
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
ROOT="${SOVEREIGN_WORK_ROOT:-$HOME/src/sovereign-os/sovereign-work}"
pass(){ echo "PASS $1"; }
soft(){ echo "SOFT $1"; }
test -f "$HOME/.local/share/sovereign/hub-ui/index.html" && pass hub_ui || soft hub_ui
test -f "$ROOT/docs/os2.0/SPEC_COVERAGE_HOST.md" && pass spec_md || soft spec_md
test -x "$ROOT/scripts/smoke-governed-loop.sh" && pass smoke_loop || soft smoke_loop
exit 0
