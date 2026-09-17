#!/usr/bin/env bash
# P4 status: Nix full VM (supported) vs empty/bootable qcow2
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ok=0

echo "=== Sovereign P4 status ==="
echo "root=$ROOT"

if [[ -e "$ROOT/dist/sovereign-vm-result" ]]; then
  echo "[OK] full system.build.vm result: dist/sovereign-vm-result"
  ok=1
else
  echo "[--] dist/sovereign-vm-result missing"
  echo "     build: bash scripts/build-sovereign-vm.sh full"
fi

if [[ -x "$ROOT/scripts/run-sovereign-vm-ssh.sh" ]]; then
  echo "[OK] SSH launcher: scripts/run-sovereign-vm-ssh.sh"
  ok=1
else
  echo "[--] scripts/run-sovereign-vm-ssh.sh missing"
fi

if [[ -x "$ROOT/scripts/boot-installed-qcow2.sh" ]]; then
  echo "[OK] boot-installed-qcow2.sh present (needs bootloader-on-disk)"
else
  echo "[--] boot-installed-qcow2.sh missing"
fi

DISK="${HOME}/sov-vm/sovereign-installed.qcow2"
if [[ -f "$DISK" ]]; then
  echo "[..] qcow2 exists: $DISK"
  if command -v qemu-img >/dev/null; then
    qemu-img info "$DISK" 2>/dev/null | head -8 || true
  fi
  echo "[NOTE] Empty/new qcow2 is NOT a bootable OS disk."
  echo "       Nix system.build.vm boots kernel+initrd from the Nix store;"
  echo "       daily path: bash scripts/run-sovereign-vm-ssh.sh full"
  echo "       True -boot c needs ISO install + bootloader on the disk (path B)."
else
  echo "[--] no $DISK (optional)"
fi

echo "=== summary ==="
if [[ "$ok" -eq 1 ]]; then
  echo "Supported installed-style path: Nix full VM + SSH (P1-3 done)."
  echo "True bootable qcow2: still open until path-B nixos-install completes."
  exit 0
fi
echo "Full VM artifacts missing — rebuild full VM first."
exit 1
