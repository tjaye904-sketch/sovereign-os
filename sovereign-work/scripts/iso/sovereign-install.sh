#!/usr/bin/env bash
# Minimal unattended-ish install sketch (Phase XVIII).
# WARNING: destructive — targets a block device you pass explicitly.
#
# Usage (from live ISO, as root):
#   sovereign-install.sh /dev/sdX
#
# Requires: parted, mkfs.ext4, rsync, nixos-install OR manual copy of running system.
# This script documents the intended flow; full nixos-install integration is next.
set -euo pipefail
DISK="${1:-}"
if [[ -z "$DISK" || ! -b "$DISK" ]]; then
  echo "usage: $0 /dev/sdX   # DESTROYS PARTITION TABLE ON DISK"
  exit 1
fi
echo "Sovereign install target: $DISK"
echo "Full automated install is not enabled in this revision."
echo "Recommended path today:"
echo "  1. Boot live ISO"
echo "  2. sudo su"
echo "  3. Follow NixOS manual install, include sovereign packages from flake"
echo "  4. Or: rebuild flake as nixosConfiguration for installed system"
echo "Aborting without writing disks."
exit 2
