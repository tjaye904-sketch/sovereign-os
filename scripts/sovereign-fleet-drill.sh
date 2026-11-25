#!/usr/bin/env bash
set -euo pipefail

CLI="${SOVEREIGN_CLI:-$(pwd)/build/apps/cli/sovereign-cli}"
ROOT="$(mktemp -d /tmp/sov-fleet.XXXXXX)"
trap 'rm -rf "$ROOT"' EXIT

SOVEREIGN_DATA_ROOT="$ROOT" "$CLI" status >/dev/null
for pair in "development:node-dev-1" "research:node-res-1" "security:node-sec-1" "knowledge:node-kno-1"; do
  ws="${pair%%:*}"
  nd="${pair##*:}"
  SOVEREIGN_DATA_ROOT="$ROOT" "$CLI" node-register-local "$nd" >/dev/null
  SOVEREIGN_DATA_ROOT="$ROOT" "$CLI" node-heartbeat "$nd" >/dev/null
  SOVEREIGN_DATA_ROOT="$ROOT" "$CLI" node-bind "$nd" "$ws" >/dev/null
  SOVEREIGN_DATA_ROOT="$ROOT" "$CLI" node-caps "$nd" host.computer_use,host.purpose >/dev/null
done

SOVEREIGN_DATA_ROOT="$ROOT" "$CLI" fleet-status --json
SOVEREIGN_DATA_ROOT="$ROOT" "$CLI" node-replace --json research node-res-2
SOVEREIGN_DATA_ROOT="$ROOT" "$CLI" plan-route --json research browser_open | grep -q '"node":"node-res-2"'
echo "FLEET_DRILL_OK"
