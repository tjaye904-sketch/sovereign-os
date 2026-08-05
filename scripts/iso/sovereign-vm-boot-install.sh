#!/usr/bin/env bash
# OS 2.0 E.2 — Boot live ISO with a qcow2 as second disk (install target).
# Does not write host /dev/* . Interactive QEMU window by default.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
ISO="${SOVEREIGN_ISO:-$ROOT/dist/Sovereign-OS-nix-amd64.iso}"
VM_DISK=""
MEM="${SOVEREIGN_QEMU_MEM:-4096}"
SMP="${SOVEREIGN_QEMU_SMP:-4}"
GRAPHIC=1

usage() {
  cat <<'U'
Usage:
  sovereign-vm-boot-install.sh --vm-disk PATH [--iso PATH] [--nographic]

Boots Sovereign live ISO with PATH as virtio disk (install target).
PATH must be a file (qcow2), never under /dev/.
U
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --vm-disk) VM_DISK="${2:-}"; shift 2 ;;
    --iso) ISO="${2:-}"; shift 2 ;;
    --nographic) GRAPHIC=0; shift ;;
    -h|--help) usage; exit 0 ;;
    *) echo "unknown: $1" >&2; usage; exit 1 ;;
  esac
done

log() { echo "[vm-boot-install] $*"; }

if [[ -z "$VM_DISK" ]]; then
  usage
  exit 1
fi
if [[ "$VM_DISK" == /dev/* ]]; then
  log "REFUSED: --vm-disk under /dev"
  exit 2
fi
if [[ ! -f "$VM_DISK" ]]; then
  log "missing qcow2: $VM_DISK"
  log "Create: bash scripts/iso/sovereign-install.sh --vm-disk $VM_DISK --confirm CREATE_VM_DISK"
  exit 1
fi
if [[ ! -f "$ISO" ]]; then
  log "missing ISO: $ISO"
  log "Build: bash scripts/build-sovereign-nix-iso.sh"
  exit 1
fi
need() { command -v "$1" >/dev/null || { log "missing: $1"; exit 1; }; }
need qemu-system-x86_64

log "ISO=$ISO"
log "DISK=$VM_DISK"
log "Inside live system: partition disk if needed, then nixos-install or follow PHASE_E doc"
log "Login: sovereign / sovereign"

ARGS=(
  -machine q35
  -m "$MEM"
  -smp "$SMP"
  -cdrom "$ISO"
  -boot d
  -drive "file=$VM_DISK,if=virtio,format=qcow2,cache=writeback"
  -device virtio-net-pci,netdev=n0
  -netdev user,id=n0,hostfwd=tcp::2222-:22
)

if [[ "$GRAPHIC" -eq 0 ]]; then
  ARGS+=(-nographic -serial mon:stdio)
else
  ARGS+=(-enable-kvm)
fi

exec qemu-system-x86_64 "${ARGS[@]}"
