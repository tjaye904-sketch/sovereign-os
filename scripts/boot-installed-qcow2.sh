#!/usr/bin/env bash
# Boot a persistent installed-style qcow2 with SSH on :2222 (no ISO).
set -euo pipefail
DISK="${1:-$HOME/sov-vm/sovereign-installed.qcow2}"
MEM="${SOVEREIGN_QEMU_MEM:-4096}"
SMP="${SOVEREIGN_QEMU_SMP:-4}"

if [[ ! -f "$DISK" ]]; then
  echo "missing disk: $DISK" >&2
  echo "Create/persist: bash scripts/persist-sovereign-vm-disk.sh" >&2
  exit 1
fi
if [[ "$DISK" == /dev/* ]]; then
  echo "REFUSED: will not boot host block devices" >&2
  exit 2
fi

need() { command -v "$1" >/dev/null || { echo "missing $1" >&2; exit 1; }; }
need qemu-system-x86_64

echo "[boot-installed] disk=$DISK"
echo "[boot-installed] ssh -p 2222 sovereign@127.0.0.1"
exec qemu-system-x86_64 \
  -machine q35 \
  -m "$MEM" \
  -smp "$SMP" \
  -enable-kvm \
  -drive "file=$DISK,if=virtio,format=qcow2,cache=writeback" \
  -boot c \
  -device virtio-net-pci,netdev=n0 \
  -netdev user,id=n0,hostfwd=tcp:127.0.0.1:2222-:22 \
  -nographic
