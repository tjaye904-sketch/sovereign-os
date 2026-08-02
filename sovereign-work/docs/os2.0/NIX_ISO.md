# Sovereign OS — NixOS ISO

## Verified on Pop!_OS host (2026-08-02)

| Check | Result |
|-------|--------|
| `bash scripts/build-sovereign-nix-iso.sh` | **PASS** → `dist/Sovereign-OS-nix-amd64.iso` |
| QEMU boot smoke | **PASS** |
| `sovereign-cli` on live image | **PASS** (`/run/current-system/sw/bin/sovereign-cli`) |
| `sovereign-cli status` | **PASS** |
| `sovereign-cli inspect coverage` | **PASS** (transparency report) |

## Build / boot

```bash
cd ~/src/sovereign-os/sovereign-work
bash scripts/build-sovereign-nix-iso.sh
bash scripts/test-sovereign-iso-boot.sh dist/Sovereign-OS-nix-amd64.iso
qemu-system-x86_64 -m 4096 -smp 4 -cdrom dist/Sovereign-OS-nix-amd64.iso -boot d
```

Live: `sovereign` / `sovereign` (rebuilds after 2026-08-02 force autologin as sovereign).

## First-boot inside VM

```bash
export SOVEREIGN_DATA_ROOT=/var/lib/sovereign   # set by image on next rebuild
sovereign-cli bootstrap owner 'change-me'
sovereign-cli login owner 'change-me'
sovereign-cli brief
sovereign-cli inspect
```
