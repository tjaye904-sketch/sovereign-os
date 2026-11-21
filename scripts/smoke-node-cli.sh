#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/src/sovereign-os/sovereign-work}"
CLI="$ROOT/build/apps/cli/sovereign-cli"
[[ -x "$CLI" ]] || CLI=$(command -v sovereign-cli)
DATA=$(mktemp -d)
export SOVEREIGN_DATA_ROOT="$DATA"
"$CLI" node-register-local laptop
"$CLI" node-register-peer edge-1 10.0.0.2:9000
"$CLI" node-heartbeat laptop | grep -q healthy
"$CLI" node-bind laptop development
"$CLI" node-caps laptop host.cli,model.chat
"$CLI" node-list | grep -q laptop
"$CLI" node-events | head -3
rm -rf "$DATA"
echo "smoke-node-cli: OK (L2.1-L2.5)"
