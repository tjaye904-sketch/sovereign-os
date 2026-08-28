# Priority 4 — Persistent installed qcow2

## Path A (recommended): reuse full system.build.vm

```bash
# 1) Boot full VM once so a disk is created, then shut down (poweroff in guest)
bash scripts/run-sovereign-vm-ssh.sh full
# in guest: sudo poweroff

# 2) Persist disk
bash scripts/persist-sovereign-vm-disk.sh

# 3) Boot persistent image (SSH :2222)
bash scripts/boot-installed-qcow2.sh
# host: ssh -p 2222 sovereign@127.0.0.1
```

## Path B: live ISO + empty qcow2 (manual nixos-install)

```bash
bash scripts/iso/sovereign-vm-boot-install.sh --vm-disk "$HOME/sov-vm/sovereign-installed.qcow2"
```

No host `/dev/sd*` writes.
