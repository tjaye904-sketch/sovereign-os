#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TARGET="${1:-minimal}"
LINK="$ROOT/dist/sovereign-vm-minimal-result"
[[ "$TARGET" == "full" ]] && LINK="$ROOT/dist/sovereign-vm-result"

if [[ ! -e "$LINK" ]]; then
  echo "Build first: bash scripts/build-sovereign-vm.sh ${TARGET}" >&2
  exit 1
fi
RUNNER=$(find -L "$LINK" -type f -name 'run-*-vm' 2>/dev/null | head -1)
if [[ -z "${RUNNER:-}" ]]; then
  echo "No run-*-vm under $LINK" >&2
  find -L "$LINK" -maxdepth 4 -type f | head -30
  exit 1
fi
echo "[vm] starting $RUNNER"
exec "$RUNNER" "$@"
