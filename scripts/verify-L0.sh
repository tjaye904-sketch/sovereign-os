#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/src/sovereign-os/sovereign-work}"
fail=0
need() { [[ -f "$1" ]] && echo "PASS $1" || { echo "FAIL $1"; fail=1; }; }
need "$ROOT/docs/os2.0/MASTER_BUILD_PLAN.md"
need "$ROOT/docs/os2.0/COMPONENT_REGISTRY.yaml"
need "$ROOT/docs/os2.0/PROFILES.md"
need "$ROOT/docs/os2.0/LEAP_LOG.md"
# 16 workspace ids in registry
c=$(grep -c 'id: ' "$ROOT/docs/os2.0/COMPONENT_REGISTRY.yaml" | head -1 || true)
w=$(grep -E '^\s+- \{ id: (development|federation)' "$ROOT/docs/os2.0/COMPONENT_REGISTRY.yaml" | wc -l)
grep -q 'id: development' "$ROOT/docs/os2.0/COMPONENT_REGISTRY.yaml" && echo PASS workspace.development || { echo FAIL development; fail=1; }
grep -q 'id: federation' "$ROOT/docs/os2.0/COMPONENT_REGISTRY.yaml" && echo PASS workspace.federation || { echo FAIL federation; fail=1; }
grep -q 'id: robotics' "$ROOT/docs/os2.0/COMPONENT_REGISTRY.yaml" && echo PASS workspace.robotics || { echo FAIL robotics; fail=1; }
n=$(grep -cE 'id: (development|knowledge|business|finance|life|health|research|security|infrastructure|automation|robotics|communications|creative|intelligence|governance|federation)' "$ROOT/docs/os2.0/COMPONENT_REGISTRY.yaml" || true)
echo "workspace_name_hits=$n (want 16)"
[[ "$n" -ge 16 ]] && echo PASS sixteen_workspaces || { echo FAIL sixteen_workspaces; fail=1; }
exit "$fail"
