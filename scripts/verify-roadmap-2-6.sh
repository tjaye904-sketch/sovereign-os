#!/usr/bin/env bash
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
ROOT="${SOVEREIGN_WORK_ROOT:-$HOME/src/sovereign-os/sovereign-work}"
fail=0
pass() { echo "PASS $1"; }
soft() { echo "SOFT $1"; }
bad()  { echo "FAIL $1"; fail=1; }

# 3 merge state
br=$(git -C "$ROOT" branch --show-current)
[[ "$br" == "main" ]] && pass "on_main" || soft "branch=$br (merge if desired)"

# 2 host maturity
if [[ -x "$ROOT/scripts/verify-host-maturity.sh" ]]; then
  bash "$ROOT/scripts/verify-host-maturity.sh" && pass "host_maturity" || bad "host_maturity"
else
  soft "verify-host-maturity missing"
fi

# 4 purposes
[[ -d "${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}/purposes" ]] && pass "purposes_dir" || bad "purposes_dir"
ls "${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}/purposes"/*.yaml >/dev/null 2>&1 && pass "purpose_yaml" || soft "purpose_yaml"

# 5 desktop
bash "$ROOT/scripts/sovereign-desktop-probe.sh" && pass "desktop_probe" || soft "desktop_probe"

# 6 cognitive cli
command -v sovereign-cognitive-cli >/dev/null && pass "cognitive_cli" || soft "cognitive_cli_not_in_path"
"$ROOT/scripts/sovereign-cognitive-cli.sh" help >/dev/null && pass "cognitive_help"

# enrichment still honest
if command -v sovereign-cli >/dev/null; then
  out=$(sovereign-cli plan "roadmap verify" 2>&1 || true)
  echo "$out" | grep -q 'enrichment=model_enriched\|enrichment=heuristic\|enrichment=model_unavailable' && pass "plan_status" || soft "plan_status"
fi

exit "$fail"
