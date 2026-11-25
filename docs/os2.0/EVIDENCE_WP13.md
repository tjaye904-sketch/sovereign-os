# Evidence — WP-13 (OS 2.0 NixOS ISO/VM Lab Build & Boot Verification)

- **Branch / Leap:** `leap/L24-nixos-iso-vm-boot-verification`
- **Flake targets evaluated cleanly via Nix 2.35.1:**
  - `.#packages.x86_64-linux.iso` -> `/nix/store/cinljd0qqz2bfcy2z86lbxasg65inrpc-sovereign-os-24.11.20250630.50ab793-x86_64-linux.iso.drv`
  - `.#packages.x86_64-linux.sovereign-vm` -> `/nix/store/1yfhdsfiziq2cg3wpvmdyc8ffa2b7mpd-nixos-vm.drv`
  - `.#packages.x86_64-linux.sovereign-node-vm` -> `/nix/store/vmz0x1np57909y60pqyjmzj44ma0h2yq-nixos-vm.drv`
  - `.#packages.x86_64-linux.sovereign-vm-minimal` -> `/nix/store/vyyk9lpshdczgq76id282b5gsgkzd0ci-nixos-vm.drv`
- **Boot & ISO Tooling Verified:** `qemu-system-x86_64 8.2.2`, `xorriso 1.5.6`, `scripts/verify-nixos-iso-vm-boot.sh`, `sovereign-cli os2-lab-status --json`, and Mission Control `GET /api/os2-lab`.
