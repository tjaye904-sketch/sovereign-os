# Sovereign OS — Boot Guide

## A. Bare-metal PC (ISO / USB)

```bash
# On a Debian/Ubuntu build host WITH network and sudo:
sudo bash scripts/build-sovereign-iso.sh ./dist/Sovereign-OS-amd64.iso
sudo dd if=./dist/Sovereign-OS-amd64.iso of=/dev/sdX bs=4M status=progress oflag=sync
# Reboot from USB
```

See `iso/README.md`.

## B. Hosted runtime (existing Linux)

```bash
./BOOT.sh
bash scripts/sovereign-ctl.sh bootstrap owner 'secret'
bash scripts/sovereign-ctl.sh start 60
```
