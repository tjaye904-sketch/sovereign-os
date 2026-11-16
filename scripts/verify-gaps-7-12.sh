#!/usr/bin/env bash
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
ROOT="${SOVEREIGN_WORK_ROOT:-$HOME/src/sovereign-os/sovereign-work}"
fail=0
pass(){ echo "PASS $1"; }
soft(){ echo "SOFT $1"; }
bad(){ echo "FAIL $1"; fail=1; }

[[ "$(git -C "$ROOT" branch --show-current)" == "main" ]] && pass "on_main" || soft "not_main"
command -v sovereign-plan >/dev/null && pass "shim_plan" || soft "shim_plan"
command -v sovereign-cognitive-cli >/dev/null && pass "cognitive_cli" || soft "cognitive_cli"
bash "$ROOT/scripts/sovereign-desktop-probe-v2.sh" >/tmp/desk-probe.out 2>&1 && pass "desktop_v2" || soft "desktop_v2"
[[ -f "${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}/gateway/auth_token" ]] && pass "gateway_token" || soft "gateway_token"
ctest --test-dir "$ROOT/build" -R 'evolution' -N 2>/dev/null | grep -q evolution && pass "evolution_tests_listed" || soft "evolution_tests"
test -x "$ROOT/scripts/purpose-run-unified.sh" && pass "purpose_unified" || bad "purpose_unified"
exit "$fail"
