# Bare-Metal Sovereign OS

## Owner requirement

Bootable disk/ISO that starts a PC as Sovereign OS.

## Architecture decision (ideal per hardware reality)

A pure from-scratch kernel is not viable for production PC hardware. Sovereign uses:

| Layer | Technology |
|-------|------------|
| Bootloader | GRUB2 + ISOLINUX (BIOS/UEFI) |
| Kernel | Linux (Debian live) |
| Init | systemd → `sovereign.target` / multi-user |
| Product OS | Sovereign daemon + CLI + durable stores |

Human authority, local-first, and governance rules remain unchanged in userland.

## Build artifact

`scripts/build-sovereign-iso.sh` → `Sovereign-OS-amd64.iso`
