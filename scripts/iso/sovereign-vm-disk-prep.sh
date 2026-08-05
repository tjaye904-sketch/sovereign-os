#!/usr/bin/env bash
# OS 2.0 E.1 — partition a VM qcow2 only (never host /dev/*).
set -euo pipefail

VM_DISK=""
CONFIRM=""
SIZE_G="${SOVEREIGN_VM_DISK_G:-40}"

usage() {
  cat <<'U'
Usage:
  sovereign-vm-disk-prep.sh --vm-disk PATH --confirm PARTITION_VM_DISK

Partitions GPT with one ext4 partition labeled "nixos".
Requires: qemu-img; guestfish (libguestfs-tools) recommended.
Refuses any path under /dev/.
U
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --vm-disk) VM_DISK="${2:-}"; shift 2 ;;
    --confirm) CONFIRM="${2:-}"; shift 2 ;;
    -h|--help) usage; exit 0 ;;
    *) echo "unknown: $1" >&2; usage; exit 1 ;;
  esac
done

log() { echo "[vm-disk-prep] $*"; }

if [[ -z "$VM_DISK" ]]; then
  usage
  exit 1
fi
if [[ "$CONFIRM" != "PARTITION_VM_DISK" ]]; then
  log "REFUSED: need --confirm PARTITION_VM_DISK"
  exit 2
fi
if [[ "$VM_DISK" == /dev/* ]]; then
  log "REFUSED: path under /dev is host block device territory"
  exit 2
fi

mkdir -p "$(dirname "$VM_DISK")"
if [[ ! -f "$VM_DISK" ]]; then
  if ! command -v qemu-img >/dev/null 2>&1; then
    log "qemu-img missing"
    exit 1
  fi
  log "creating qcow2 ${SIZE_G}G at $VM_DISK"
  qemu-img create -f qcow2 "$VM_DISK" "${SIZE_G}G"
fi

if ! command -v guestfish >/dev/null 2>&1; then
  log "guestfish not found — install: sudo apt install libguestfs-tools"
  log "Manual fallback (inside live ISO + second virtio disk):"
  log "  parted /dev/vdb -- mklabel gpt"
  log "  parted /dev/vdb -- mkpart primary ext4 1MiB 100%"
  log "  mkfs.ext4 -L nixos /dev/vdb1"
  log "E.1 partial: image exists; partition deferred"
  exit 0
fi

log "partitioning with guestfish: $VM_DISK"
guestfish --rw -a "$VM_DISK" <<'GF'
run
part-init /dev/sda gpt
part-add /dev/sda p 2048 -2048
mkfs ext4 /dev/sda1
set-label /dev/sda1 nixos
GF

log "DONE: GPT + ext4 label=nixos on $VM_DISK"
qemu-img info "$VM_DISK" | head -15
exit 0
