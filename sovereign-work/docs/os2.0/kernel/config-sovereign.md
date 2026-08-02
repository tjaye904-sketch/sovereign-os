# Reproducible Sovereign kernel configuration (Linux)

Target: Debian bookworm `linux-image-amd64` baseline + hardening.

## Required modules / features

- EFI stub, TPM 2.0 (`CONFIG_TCG_TPM`, `CONFIG_HW_RANDOM_TPM`)
- dm-crypt, LUKS, overlayfs, squashfs (live)
- cgroup v2, namespaces (sandboxing)
- IPv4/IPv6, WireGuard
- USB, NVMe, SATA, virtio
- AppArmor or SELinux (install-time choice; default AppArmor)

## Hardening (intent)

- Kernel lockdown (integrity mode when Secure Boot)
- `kernel.kptr_restrict=2`, `dmesg_restrict=1`
- Verified boot chain: firmware → shim → GRUB → signed kernel → dm-verity root (install option)

## Version pin

Document exact `uname -r` in each ISO release notes under `dist/RELEASE_NOTES`.
