#!/usr/bin/env bash
# Launch built NixOS VM. Usage: run-sovereign-vm.sh [minimal|full]
# Do NOT forward the target name to qemu (it is not a disk image).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TARGET="${1:-minimal}"
case "$TARGET" in
  minimal|full) ;;
  *)
    echo "usage: $0 [minimal|full]" >&2
    exit 1
    ;;
esac

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
echo "[vm] target=$TARGET"
echo "[vm] starting $RUNNER"
# No extra args — qemu runner manages its own disk image
exec "$RUNNER"
