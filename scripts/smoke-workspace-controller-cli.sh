#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/src/sovereign-os/sovereign-work}"
CLI="$ROOT/build/apps/cli/sovereign-cli"
[[ -x "$CLI" ]] || CLI=$(command -v sovereign-cli)
DATA=$(mktemp -d)
export SOVEREIGN_DATA_ROOT="$DATA"
"$CLI" workspace-create ctrl-dev development
"$CLI" workspace-status ctrl-dev | grep -q 'status=active'
"$CLI" workspace-suspend ctrl-dev | grep -q 'status=suspended'
"$CLI" workspace-status | grep -q 'ctrl-dev'
"$CLI" workspace-activate ctrl-dev | grep -q 'status=active'
rm -rf "$DATA"
echo "smoke-workspace-controller-cli: OK"
