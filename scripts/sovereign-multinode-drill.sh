#!/usr/bin/env bash
set -euo pipefail

CLI="${1:-${SOVEREIGN_CLI:-$(cd "$(dirname "${BASH_SOURCE}")/.." && pwd)/build/apps/cli/sovereign-cli}}"
ROOT="$(mktemp -d /tmp/sov-l31-multinode.XXXXXX)"
trap 'rm -rf "$ROOT"' EXIT
export SOVEREIGN_DATA_ROOT="$ROOT"

"$CLI" status >/dev/null
"$CLI" capability-seed >/dev/null
"$CLI" capability-enable host.sys >/dev/null
"$CLI" node-register-local core-pop-os >/dev/null
"$CLI" node-heartbeat core-pop-os >/dev/null
"$CLI" node-register-peer edge-nixos-node http://127.0.0.1:8769 >/dev/null
"$CLI" node-heartbeat edge-nixos-node >/dev/null
"$CLI" node-bind edge-nixos-node research >/dev/null
"$CLI" node-caps edge-nixos-node host.sys >/dev/null

"$CLI" plan-route --json research sys | grep -q '"node":"edge-nixos-node"'
"$CLI" node-execute --json research sys "collect telemetry snapshot" | grep -q '"executed":true'
"$CLI" audit-list | grep -q "federation"

echo "MULTINODE_ENROLLMENT_DRILL_OK core=core-pop-os edge=edge-nixos-node"
