#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CLI="${1:-$ROOT/build/sovereign-cli}"
if [[ ! -x "$CLI" ]]; then
  bash "$ROOT/scripts/build-sovereign-cli.sh" "$CLI"
fi
DATA=$(mktemp -d)
export SOVEREIGN_DATA_ROOT="$DATA"
trap 'rm -rf "$DATA"' EXIT
echo "[e2e] data_root=$DATA"
"$CLI" bootstrap owner 'e2e-secret'
"$CLI" login owner 'e2e-secret'
"$CLI" workspace-create e2e development
"$CLI" memory-add e2e 'e2e memory payload'
"$CLI" chat "ping" 2>/dev/null || echo "[e2e] model offline (allowed)"
DEC=$("$CLI" recommend ship 'e2e action' | sed -n 's/governance_decision_id=//p')
test -n "$DEC"
"$CLI" approve "$DEC" 'e2e approve'
"$CLI" execute ship "$DEC"
"$CLI" inspect audit | head -20
"$CLI" inspect coverage | head -15
"$CLI" status | grep -q "data_root="
"$CLI" memory-list | grep -q 'e2e memory'
"$CLI" workspace-list | grep -q e2e
echo "e2e-temp-dataroot: OK"
