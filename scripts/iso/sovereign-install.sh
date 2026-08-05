#!/usr/bin/env bash
# Sovereign installer gate (Milestone B.1)
# Default: dry-run. Never writes block devices without explicit --confirm.
set -euo pipefail

MODE="dry-run"
DISK=""
VM_DISK=""
CONFIRM=""

usage() {
  cat <<'U'
Usage:
  sovereign-install.sh --dry-run
  sovereign-install.sh --vm-disk PATH --confirm CREATE_VM_DISK
  sovereign-install.sh --disk /dev/sdX --confirm DESTROY_DISK   # B.1: still aborts after checks

Environment:
  SOVEREIGN_INSTALL_ALLOW_HOST=1  required together with DESTROY_DISK for host path (future)
U
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --dry-run) MODE="dry-run"; shift ;;
    --vm-disk) MODE="vm"; VM_DISK="${2:-}"; shift 2 ;;
    --disk) MODE="host"; DISK="${2:-}"; shift 2 ;;
    --confirm) CONFIRM="${2:-}"; shift 2 ;;
    -h|--help) usage; exit 0 ;;
    *)
      # legacy: bare /dev path
      if [[ -b "${1:-}" ]]; then
        MODE="host"
        DISK="$1"
        shift
      else
        echo "unknown arg: $1" >&2
        usage
        exit 1
      fi
      ;;
  esac
done

log() { echo "[sovereign-install] $*"; }

case "$MODE" in
  dry-run)
    log "DRY-RUN: no disks will be written"
    log "Would stage: partition → mkfs → nixos-install (sovereign-system) → first-boot"
    log "Next: --vm-disk ~/sov-vm/sovereign.qcow2 --confirm CREATE_VM_DISK"
    exit 0
    ;;
  vm)
    if [[ -z "$VM_DISK" ]]; then
      echo "missing --vm-disk PATH" >&2
      exit 1
    fi
    if [[ "$CONFIRM" != "CREATE_VM_DISK" ]]; then
      log "REFUSED: need --confirm CREATE_VM_DISK"
      exit 2
    fi
    # Refuse if path looks like a raw host disk node
    if [[ "$VM_DISK" == /dev/* ]]; then
      log "REFUSED: --vm-disk must not be under /dev (use --disk for block devices)"
      exit 2
    fi
    mkdir -p "$(dirname "$VM_DISK")"
    if [[ -e "$VM_DISK" ]]; then
      log "VM disk already exists: $VM_DISK (not overwriting)"
      ls -lh "$VM_DISK"
      exit 0
    fi
    if ! command -v qemu-img >/dev/null 2>&1; then
      log "qemu-img not found; install qemu-utils to create qcow2"
      exit 1
    fi
    qemu-img create -f qcow2 "$VM_DISK" 40G
    log "Created empty VM disk: $VM_DISK"
    log "Empty qcow2 ready. Next (OS 2.0 E.1): scripts/iso/sovereign-vm-disk-prep.sh --vm-disk ... --confirm PARTITION_VM_DISK"
    exit 0
    ;;
  host)
    if [[ -z "$DISK" ]]; then
      echo "missing --disk /dev/..." >&2
      exit 1
    fi
    if [[ "$CONFIRM" != "DESTROY_DISK" ]]; then
      log "REFUSED: host install requires --confirm DESTROY_DISK"
      exit 2
    fi
    if [[ "${SOVEREIGN_INSTALL_ALLOW_HOST:-}" != "1" ]]; then
      log "REFUSED: set SOVEREIGN_INSTALL_ALLOW_HOST=1 for host installs (not enabled in B.1)"
      exit 2
    fi
    if [[ ! -b "$DISK" ]]; then
      log "REFUSED: not a block device: $DISK"
      exit 2
    fi
    log "B.1: host destructive install is designed but NOT executed"
    log "Would destroy partition table on $DISK — aborting without write"
    exit 2
    ;;
  *)
    usage
    exit 1
    ;;
esac
