#!/usr/bin/env bash
# Boot VM with SSH on localhost:2222
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TARGET="${1:-full}"
case "$TARGET" in minimal|full) ;; *)
  echo "usage: $0 [minimal|full]" >&2; exit 1
esac

LINK="$ROOT/dist/sovereign-vm-minimal-result"
[[ "$TARGET" == "full" ]] && LINK="$ROOT/dist/sovereign-vm-result"

if [[ ! -e "$LINK" ]]; then
  echo "Build first: bash scripts/build-sovereign-vm.sh ${TARGET}" >&2
  exit 1
fi
RUNNER=$(find -L "$LINK" -type f -name 'run-*-vm' 2>/dev/null | head -1)
[[ -n "${RUNNER:-}" ]] || { echo "No run-*-vm under $LINK" >&2; exit 1; }

export QEMU_NET_OPTS="hostfwd=tcp:127.0.0.1:2222-:22"

echo "[vm-ssh] target=$TARGET"
echo "[vm-ssh] ssh -p 2222 sovereign@127.0.0.1   password: sovereign"
echo "[vm-ssh] starting $RUNNER"
exec "$RUNNER"
