#!/usr/bin/env bash
set -euo pipefail

REL_DIR="${SOVEREIGN_RELEASE_DIR:-$HOME/src/sovereign-os/releases}"
ISO_FILE="${SOVEREIGN_ISO_FILE:-$(ls -t "$REL_DIR"/sovereign-os-2.0-*-x86_64.iso 2>/dev/null | head -n 1)}"

usage() {
  echo "Usage:"
  echo "  $0 --list                     # List removable USB target devices & verify ISO SHA256"
  echo "  $0 --yes-flash /dev/sdX       # Verify SHA256SUMS and flash ISO to removable USB /dev/sdX"
}

if [[ -z "${ISO_FILE:-}" || ! -f "$ISO_FILE" ]]; then
  echo "ERROR: No release ISO found in $REL_DIR" >&2
  exit 1
fi

if [[ -f "$REL_DIR/SHA256SUMS" ]]; then
  (cd "$REL_DIR" && sha256sum -c SHA256SUMS)
fi

if [[ "${1:-}" == "--list" || $# -eq 0 ]]; then
  echo "Verified ISO: $ISO_FILE ($(du -h "$ISO_FILE" | awk '{print $1}'))"
  echo "Removable USB block devices:"
  lsblk -d -o NAME,TRAN,RM,SIZE,MODEL | awk 'NR==1 || ($2=="usb" && $3=="1") || $2=="mmc" || $1 ~ /^mmcblk/'
  exit 0
fi

if [[ "${1:-}" != "--yes-flash" || -z "${2:-}" ]]; then
  usage
  exit 2
fi

DEV="$2"
[[ -b "$DEV" ]] || { echo "ERROR: $DEV is not a block device" >&2; exit 1; }
BASE="$(basename "$DEV")"
TRAN="$(lsblk -dn -o TRAN "$DEV" 2>/dev/null || true)"
RM="$(lsblk -dn -o RM "$DEV" 2>/dev/null | tr -d ' ')"
MOUNTED="$(lsblk -nr -o MOUNTPOINT "$DEV" | grep -v '^$' || true)"

if [[ ! ( ( "$TRAN" == "usb" && "$RM" == "1" ) || "$TRAN" == "mmc" || "$BASE" == mmcblk* ) ]]; then
  echo "REFUSED: $DEV is not a removable USB device (TRAN=$TRAN RM=$RM)" >&2
  exit 1
fi
if [[ -n "$MOUNTED" ]]; then
  echo "REFUSED: $DEV has mounted partitions ($MOUNTED); unmount first" >&2
  exit 1
fi

echo "Flashing $ISO_FILE -> $DEV ($BASE)..."
sudo dd if="$ISO_FILE" of="$DEV" bs=4M status=progress conv=fsync
sync
echo "USB_FLASH_COMPLETE device=$DEV iso=$(basename "$ISO_FILE")"
