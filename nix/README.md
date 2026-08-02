# Sovereign OS — NixOS ISO

This directory defines a **NixOS live/installer image** that boots as Sovereign OS.

## Architecture

| Layer | Technology |
|-------|------------|
| Bootloader | GRUB (BIOS + EFI) via NixOS ISO machinery |
| Kernel | Linux from nixpkgs 24.11 |
| Userspace | NixOS minimal installer base |
| Product | `sovereign-cli` + `sovereign-daemon` (built from this repo) |
| Orchestration | systemd → `sovereign-daemon` |

## Build (on Pop!_OS / any Linux with Nix)

```bash
# Install Nix if needed: https://nixos.org/download
# Enable flakes (~/.config/nix/nix.conf):
#   experimental-features = nix-command flakes

cd /path/to/sovereign-work
bash scripts/build-sovereign-nix-iso.sh
```

Output: `dist/Sovereign-OS-nix-amd64.iso`

## Boot test (QEMU)

```bash
sudo apt install -y qemu-system-x86   # once
bash scripts/test-sovereign-iso-boot.sh dist/Sovereign-OS-nix-amd64.iso
```

## Manual QEMU

```bash
qemu-system-x86_64 -m 4096 -smp 4 -cdrom dist/Sovereign-OS-nix-amd64.iso -boot d
```

Live credentials: `sovereign` / `sovereign` (change immediately).
