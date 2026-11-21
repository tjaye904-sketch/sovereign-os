#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/src/sovereign-os/sovereign-work}"
CLI="$ROOT/build/apps/cli/sovereign-cli"
[[ -x "$CLI" ]] || CLI=$(command -v sovereign-cli)
DATA=$(mktemp -d)
export SOVEREIGN_DATA_ROOT="$DATA"
"$CLI" capability-seed | grep -q seeded_added
"$CLI" capability-list | grep -q host.cli
"$CLI" capability-disable host.computer_use | grep -q 'enabled=0'
"$CLI" capability-check host.computer_use | grep -q 'enabled=0'
"$CLI" capability-enable host.computer_use | grep -q 'enabled=1'
rm -rf "$DATA"
echo "smoke-capability-cli: OK"
