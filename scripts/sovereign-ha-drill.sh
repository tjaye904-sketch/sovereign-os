#!/usr/bin/env bash
set -euo pipefail

CLI="${SOVEREIGN_CLI:-$(pwd)/build/apps/cli/sovereign-cli}"
SRC_ROOT="$(mktemp -d /tmp/sov-ha-src.XXXXXX)"
SNAP_DIR="$(mktemp -d /tmp/sov-ha-snap.XXXXXX)"
DST_ROOT="$(mktemp -d /tmp/sov-ha-dst.XXXXXX)"
trap 'rm -rf "$SRC_ROOT" "$SNAP_DIR" "$DST_ROOT"' EXIT

SOVEREIGN_DATA_ROOT="$SRC_ROOT" "$CLI" status >/dev/null
SOVEREIGN_DATA_ROOT="$SRC_ROOT" "$CLI" bootstrap ha-admin "ha-secret-pass" >/dev/null
SOVEREIGN_DATA_ROOT="$SRC_ROOT" "$CLI" node-register-local ha-primary >/dev/null
SOVEREIGN_DATA_ROOT="$SRC_ROOT" "$CLI" core-backup --json "$SNAP_DIR"
SOVEREIGN_DATA_ROOT="$DST_ROOT" "$CLI" core-restore --json "$SNAP_DIR"
SOVEREIGN_DATA_ROOT="$DST_ROOT" "$CLI" node-list | grep -q "ha-primary"
echo "HA_DRILL_OK"
