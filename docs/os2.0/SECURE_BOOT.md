# OS 2.0 — Secure Boot & measured boot (F.1)

## Status

| Item | State |
|------|--------|
| Live ISO boots (UEFI/BIOS via NixOS) | **Working** (D.2 evidence) |
| Sovereign product chain (identity → audit) | **Working** |
| Platform Secure Boot keys (PK/KEK/db) enrolled | **Not done** (operator-owned) |
| Measured boot / TPM PCR policy | **Not done** |
| Secure Boot **required** for Sovereign runtime | **Not enforced** |

This document is the **checklist and design**. F.1 does **not** enroll keys or flash firmware.

## Goals

1. Bootloader and kernel authenticity when the machine has Secure Boot enabled.  
2. Optional later: TPM-sealed secrets for `$SOVEREIGN_DATA_ROOT`.  
3. Never weaken host install gates (E.4).

## NixOS / ISO notes

- Current image: NixOS 24.11-based live ISO (`dist/Sovereign-OS-nix-amd64.iso`).  
- Upstream NixOS supports UEFI; Secure Boot typically needs **signed** bootloader/kernel (e.g. `lanzaboote` / custom keys) — **out of tree for F.1**.  
- Live QEMU tests often run **without** Secure Boot firmware vars; PASS does not prove SB.

## Operator checklist (hardware)

When you enable Secure Boot on real hardware:

1. **Backup** current firmware keys / set Setup Mode if vendor requires.  
2. Decide trust model: Microsoft+OEM keys only vs **custom PK** (advanced).  
3. Build/sign boot chain (future flake module; not in F.1).  
4. Enroll **db** certificates that sign your systemd-boot/GRUB + kernel.  
5. Boot live ISO in UEFI mode; confirm firmware shows Secure Boot **enabled**.  
6. Install only via E.4 gates when wipe is intentional.  
7. Record evidence: firmware screenshot or `mokutil --sb-state` in an audit note.

## Host inspection commands (read-only)

```bash
# Secure Boot state (Linux host)
mokutil --sb-state 2>/dev/null || echo "mokutil not installed"

# EFI vars present?
ls /sys/firmware/efi 2>/dev/null && echo "UEFI booted" || echo "Legacy/BIOS or no EFI sysfs"

# Optional package: sudo apt install mokutil
```

## Measured boot (future)

| Step | Purpose |
|------|---------|
| TPM 2.0 present | `ls /dev/tpm0` or `tpm2_getcap` |
| PCR extend on boot | Firmware + bootloader + kernel |
| Seal data_root key to PCR policy | Only unlock if boot chain matches |
| Audit PCR quotes | Remote attestation (far future) |

**F.1 explicitly does not** seal Sovereign secrets to TPM.

## Acceptance (F.1)

- [x] Document Secure Boot vs live ISO reality  
- [x] Operator checklist without requiring key enrollment  
- [x] `scripts/verify-secure-boot-docs.sh` passes  
- [ ] (Optional later) Signed boot flake module  
- [ ] (Optional later) TPM seal for data_root  

## Relation to Sovereign identity

Secure Boot protects **boot integrity**.  
Sovereign MFA/recovery protects **operator identity**.  
They are complementary; neither replaces the other.
