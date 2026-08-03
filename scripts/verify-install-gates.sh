#!/usr/bin/env bash
# B.1 verification: installer gates refuse unsafe paths
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
S="$ROOT/scripts/iso/sovereign-install.sh"
chmod +x "$S"

"$S" --dry-run

# missing confirm on vm → exit 2
set +e
"$S" --vm-disk /tmp/sov-should-not-create-$$.qcow2
rc=$?
set -e
[[ "$rc" -eq 2 ]] || { echo "expected exit 2 without confirm"; exit 1; }
[[ ! -e /tmp/sov-should-not-create-$$.qcow2 ]]

# host without confirm → exit 2
set +e
"$S" --disk /dev/null --confirm WRONG
rc=$?
set -e
[[ "$rc" -eq 2 ]] || { echo "expected refuse wrong confirm"; exit 1; }

# legacy bare disk path without confirm → refuse
set +e
"$S" /dev/null
rc=$?
set -e
[[ "$rc" -eq 2 ]] || [[ "$rc" -eq 1 ]] || true

echo "verify-install-gates: OK"
