#!/usr/bin/env bash
# OS 2.0 E.4 — host install must still refuse to write
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
S="$ROOT/scripts/iso/sovereign-install.sh"
chmod +x "$S"

# No confirm
set +e
"$S" --disk /dev/null
rc=$?
set -e
[[ "$rc" -eq 2 ]]

# Wrong confirm
set +e
"$S" --disk /dev/null --confirm DESTROY_DISK
rc=$?
set -e
[[ "$rc" -eq 2 ]]  # ALLOW_HOST not set

# Full flags still abort without write (design only)
set +e
SOVEREIGN_INSTALL_ALLOW_HOST=1 "$S" --disk /dev/null --confirm DESTROY_DISK
rc=$?
set -e
[[ "$rc" -eq 2 ]]

# /dev path never accepted as --vm-disk
set +e
"$S" --vm-disk /dev/sda --confirm CREATE_VM_DISK
rc=$?
set -e
[[ "$rc" -eq 2 ]]

echo "verify-e4-host-gates: OK"
