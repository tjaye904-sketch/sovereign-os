#!/usr/bin/env bash
# Boot an installed qcow2 (no ISO) — OS 2.0 E.3 helper
set -euo pipefail
VM_DISK="${1:-}"
MEM="${SOVEREIGN_QEMU_MEM:-4096}"
if [[ -z "$VM_DISK" || "$VM_DISK" == /dev/* || ! -f "$VM_DISK" ]]; then
  echo "usage: $0 /path/to/installed.qcow2   # file only, not /dev/*" >&2
  exit 1
fi
exec qemu-system-x86_64 -machine q35 -m "$MEM" -smp 4 \
  -drive "file=$VM_DISK,if=virtio,format=qcow2" \
  -boot c -enable-kvm \
  -device virtio-net-pci,netdev=n0 -netdev user,id=n0,hostfwd=tcp::2222-:22
