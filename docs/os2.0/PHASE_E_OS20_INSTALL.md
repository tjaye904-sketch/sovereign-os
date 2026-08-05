# OS 2.0 — Milestone E: Installed system path (VM first)

## Goal

Move from **live ISO only** to **installed Sovereign configuration** on a disk image, then (later) host hardware under the same gates.

## Order

| Step | Scope | Host disk |
|------|--------|-----------|
| **E.1** | Create + partition VM qcow2; document install | **No** |
| **E.2** | `nixos-install` / copy into qcow2 (or build.vm root) | **No** |
| **E.3** | Boot installed image; first-boot identity | **No** |
| **E.4** | Host path with DESTROY_DISK + ALLOW_HOST (still explicit) | **Yes, gated** |

## E.1 commands

```bash
# Create empty 40G qcow2
bash scripts/iso/sovereign-install.sh \
  --vm-disk "$HOME/sov-vm/sovereign.qcow2" \
  --confirm CREATE_VM_DISK

# Partition GPT + ext4 label nixos (requires guestfish OR manual)
bash scripts/iso/sovereign-vm-disk-prep.sh \
  --vm-disk "$HOME/sov-vm/sovereign.qcow2" \
  --confirm PARTITION_VM_DISK

bash scripts/verify-vm-disk-prep.sh
```

## Standing rules (unchanged)

1. No `/dev/sd*` / `/dev/nvme*` without `DESTROY_DISK` + `SOVEREIGN_INSTALL_ALLOW_HOST=1`
2. `loop --auto-approve` never installs
3. MFA does not write disks

## Pass criteria E.1

- Dry-run / missing confirm still exit 2
- qcow2 created under home path only
- Prep either partitions via guestfish or documents fallback without writing host block devices
- CTest still green

## E.2 — Boot live ISO + VM disk

```bash
# Optional: sudo apt install -y libguestfs-tools
# bash scripts/iso/sovereign-vm-disk-prep.sh --vm-disk "$HOME/sov-vm/sovereign.qcow2" --confirm PARTITION_VM_DISK

bash scripts/iso/sovereign-vm-boot-install.sh \
  --vm-disk "$HOME/sov-vm/sovereign.qcow2"

# nographic / CI-ish:
# bash scripts/iso/sovereign-vm-boot-install.sh --vm-disk ... --nographic
```

Inside the live guest (login `sovereign` / `sovereign`):

1. `lsblk` — find virtio disk (often `/dev/vda` or `/dev/vdb`)
2. Partition + `mkfs.ext4 -L nixos` if not pre-prepped
3. Mount and `nixos-install` using flake `sovereign-system` (E.2 continues in E.3 docs)
4. Or use host-side `bash scripts/build-sovereign-vm.sh` for a prebuilt installed-style VM without manual install

Gates: `bash scripts/verify-vm-boot-install-gates.sh`
