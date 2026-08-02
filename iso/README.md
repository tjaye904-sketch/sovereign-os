# Sovereign OS — Bare-Metal Bootable ISO

## Design

Sovereign boots a PC as a **hybrid bare-metal OS**:

1. **Firmware** (BIOS/UEFI) loads GRUB/ISOLINUX from the ISO/USB  
2. **Linux kernel + initrd** (live-boot) bring up hardware  
3. **Sovereign** is the primary operating environment:  
   - `sovereign-daemon` under systemd  
   - `sovereign-cli` / autologin console  
   - durable state under `/var/lib/sovereign`

This matches specialized OS practice (secure live systems): a real kernel for hardware,
with Sovereign as the product OS surface — not a toy “OS” without drivers.

## Build (on a Debian/Ubuntu host with network)

```bash
sudo bash scripts/build-sovereign-iso.sh ./dist/Sovereign-OS-amd64.iso
```

## Boot

- Flash to USB: `sudo dd if=Sovereign-OS-amd64.iso of=/dev/sdX bs=4M status=progress oflag=sync`
- Or boot in QEMU: `qemu-system-x86_64 -m 2048 -cdrom Sovereign-OS-amd64.iso -boot d`

Default live user: `sovereign` / `sovereign` (change immediately).

## Sandbox note

The development sandbox has **no network and no ISO toolchains**, so the `.iso` binary
must be produced on your machine with the script above.
