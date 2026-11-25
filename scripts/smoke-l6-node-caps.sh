#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/src/sovereign-os/sovereign-work}"
CLI="${SOVEREIGN_CLI:-$ROOT/build/apps/cli/sovereign-cli}"
export SOVEREIGN_DATA_ROOT="${SOVEREIGN_DATA_ROOT:-$(mktemp -d)}"
echo "data_root=$SOVEREIGN_DATA_ROOT"

"$CLI" capability-seed || true
# Prefer C++ unit test if CLI not yet wired
if "$CLI" 2>&1 | grep -q node-caps-bind; then
  "$CLI" node-caps-bind demo-node does.not.exist && exit 1 || true
  CAP=$("$CLI" capability-list | awk -F= '/id=/{print $2; exit}' | awk '{print $1}')
  CAP=${CAP:-host.cli}
  "$CLI" node-caps-bind demo-node "$CAP"
  "$CLI" node-caps-list demo-node | grep -q "$CAP"
  "$CLI" node-caps-unbind demo-node "$CAP"
  echo "smoke-l6 CLI: OK"
else
  echo "CLI node-caps-* not present — relying on node_capability_binding_smoke_test"
fi
ctest --test-dir "$ROOT/build" -R node_capability_binding --output-on-failure
echo "smoke-l6: OK"
