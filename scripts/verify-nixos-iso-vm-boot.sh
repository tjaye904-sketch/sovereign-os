#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE}")/.." && pwd)"
cd "$ROOT"

grep -q 'SOVEREIGN_PROFILE = "nixos-sovereign";' nix/modules/sovereign-iso.nix
grep -q 'sovereign-release' nix/modules/sovereign-iso.nix
grep -q 'console=ttyS0,115200n8' nix/modules/sovereign-iso.nix

if command -v nix >/dev/null 2>&1; then
  for attr in iso sovereign-vm sovereign-node-vm sovereign-vm-minimal; do
    drv=$(nix eval --raw ".#packages.x86_64-linux.${attr}.drvPath")
    [[ "$drv" == /nix/store/*.drv ]] || { echo "BAD_DRV: $attr -> $drv"; exit 1; }
    echo "EVAL_OK $attr -> $drv"
  done
fi

command -v qemu-system-x86_64 >/dev/null 2>&1 && echo "QEMU_OK $(qemu-system-x86_64 --version | head -n 1)"
command -v xorriso >/dev/null 2>&1 && echo "XORRISO_OK $(xorriso --version 2>&1 | head -n 1)"
echo "NIXOS_ISO_VM_BOOT_GATE_OK"
