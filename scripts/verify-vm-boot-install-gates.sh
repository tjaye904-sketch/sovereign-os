#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
S="$ROOT/scripts/iso/sovereign-vm-boot-install.sh"
chmod +x "$S"

set +e
"$S" 2>/dev/null
rc=$?
set -e
[[ "$rc" -ne 0 ]]

set +e
"$S" --vm-disk /dev/null 2>/dev/null
rc=$?
set -e
[[ "$rc" -eq 2 ]]

set +e
"$S" --vm-disk /tmp/no-such-sov-e2.qcow2 2>/dev/null
rc=$?
set -e
[[ "$rc" -eq 1 ]]

echo "verify-vm-boot-install-gates: OK"
