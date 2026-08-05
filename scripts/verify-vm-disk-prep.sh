#!/usr/bin/env bash
# E.1 gate verification — no real partition required
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PREP="$ROOT/scripts/iso/sovereign-vm-disk-prep.sh"
chmod +x "$PREP"

# Missing confirm → 2
set +e
"$PREP" --vm-disk /tmp/sov-e1-should-not.qcow2
rc=$?
set -e
[[ "$rc" -eq 2 ]] || { echo "expected exit 2"; exit 1; }
[[ ! -e /tmp/sov-e1-should-not.qcow2 ]]

# /dev path refused
set +e
"$PREP" --vm-disk /dev/null --confirm PARTITION_VM_DISK
rc=$?
set -e
[[ "$rc" -eq 2 ]] || { echo "expected refuse /dev"; exit 1; }

# Dry path: create under /tmp with confirm (qemu-img only if no guestfish still ok)
TMP=$(mktemp -u /tmp/sov-e1-XXXX.qcow2)
"$PREP" --vm-disk "$TMP" --confirm PARTITION_VM_DISK
[[ -f "$TMP" ]]
rm -f "$TMP"

echo "verify-vm-disk-prep: OK"
