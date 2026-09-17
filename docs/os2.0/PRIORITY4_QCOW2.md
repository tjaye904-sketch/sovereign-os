# Priority 4 — Persistent installed qcow2

## Why "Boot failed: not a bootable disk" happens

NixOS `system.build.vm` does **not** put a full GRUB + root filesystem on the
qcow2 by default. QEMU is started with:

- kernel + initrd from the **Nix store**
- a qcow2 used mainly for **mutable state**

So an empty or newly created `sovereign-installed.qcow2` has no bootloader.
SeaBIOS `-boot c` correctly reports **not a bootable disk**.

| Approach | Bootable with `-boot c` alone? | Daily use |
|----------|--------------------------------|-----------|
| `run-sovereign-vm-ssh.sh full` | N/A (store kernel) | **Supported** |
| Empty qcow2 + `boot-installed-qcow2.sh` | **No** | Not supported |
| ISO + `nixos-install` into qcow2 | **Yes** (after install) | Path B |

## Path A (recommended): full system.build.vm

```bash
bash scripts/build-sovereign-vm.sh full   # once after nix changes
bash scripts/run-sovereign-vm-ssh.sh full
# host terminal:
ssh -p 2222 sovereign@127.0.0.1   # password: sovereign
```

Inside guest: `sovereign-cli status`, `systemctl status sovereign-daemon`.

## Path B: live ISO + install into qcow2

```bash
ls -lh dist/Sovereign-OS-nix-amd64.iso
qemu-img create -f qcow2 "$HOME/sov-vm/sovereign-installed.qcow2" 40G
bash scripts/iso/sovereign-vm-boot-install.sh \
  --vm-disk "$HOME/sov-vm/sovereign-installed.qcow2"
```

Then partition, `mkfs.ext4 -L nixos`, mount, and `nixos-install` from the
`sovereign-system` flake (manual, longer). **No host `/dev/sd*` writes.**

## Verify

```bash
bash scripts/verify-p4-status.sh
```

## Related

- `docs/os2.0/VM_SSH_DAEMON.md` — SSH, data root, daemon
- `docs/os2.0/PHASE_E_OS20_INSTALL.md` — install gates
