# Phase I — Kernel and Boot Environment

## Locked decisions

| Topic | Decision |
|-------|----------|
| Kernel | **Linux** (Debian stable line); reproducible `config-sovereign` |
| Bootloader | GRUB2 EFI + BIOS (ISOLINUX legacy) |
| Secure Boot | Supported via signed shim path (host-signed keys in install) |
| Live / Installer | Same ISO; install mode via kernel cmdline `sovereign.install=1` |

## Boot menu targets

| Entry | Purpose |
|-------|---------|
| Sovereign OS (live) | Default multi-user + daemon |
| Safe mode | Minimal services, no agents |
| Recovery | Read-only root diagnostics |
| Rescue shell | Single-user root shell |
| Rollback | Previous kernel/root if present |
| Memory test | memtest86+ when packaged |
| Hardware inspect | `sovereign-boot-diag` |

## Deliverables in tree

- `iso/grub/grub.cfg` — full menu  
- `scripts/boot/sovereign-boot-diag.sh` — diagnostics  
- `scripts/build-sovereign-iso.sh` — ISO builder  
- `docs/os2.0/kernel/config-sovereign.md` — required modules & hardening  
