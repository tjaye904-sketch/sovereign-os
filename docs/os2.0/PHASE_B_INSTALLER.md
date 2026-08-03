# Milestone B — Installer (non-destructive by default)

## Goal

Install Sovereign as a **primary OS configuration** first in a **VM disk image**, never on host hardware until the operator explicitly names a block device and confirms destruction.

## Standing rules (from A)

1. Revoke/MFA ≠ disk write  
2. `loop --auto-approve` never calls installer  
3. Host `/dev/sd*` / `/dev/nvme*` writes require explicit confirm  
4. Default path is **dry-run** or **VM qcow2 only**

## Stages

| Stage | B.1 (this) | B.2 | B.3 |
|-------|------------|-----|-----|
| Design + dry-run gates | **yes** | | |
| Create/attach VM qcow2 | script | | |
| Partition + filesystem in VM | | yes | |
| `nixos-install` with `sovereign-system` | | yes | |
| First-boot identity bootstrap | | | yes |
| Host bare-metal path | | | gated |

## Commands

```bash
# Always safe
bash scripts/iso/sovereign-install.sh --dry-run

# VM image only (creates empty qcow2; does not format host disks)
bash scripts/iso/sovereign-install.sh --vm-disk "$HOME/sov-vm/sovereign.qcow2" --confirm CREATE_VM_DISK

# Host disk — refused unless both flags set (still aborts in B.1 with message)
bash scripts/iso/sovereign-install.sh --disk /dev/sdX --confirm DESTROY_DISK
```

## Evidence for B.1 done

- `--dry-run` exits 0 and writes nothing  
- Missing confirm → exit 2, no write  
- CTest still green  
- Docs committed  

## Explicit non-goals for B.1

- No real partitioning  
- No `nixos-install` execution  
- No host disk modification  

## B.2 — NixOS `build.vm` (no host disks)

```bash
bash scripts/build-sovereign-vm.sh    # long first build
bash scripts/run-sovereign-vm.sh      # QEMU window
# Inside VM: sovereign-cli status / bootstrap after first boot
```

Evidence: `nix flake` exposes `packages.sovereign-vm`; build produces `run-*-vm`.
