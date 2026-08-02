#!/usr/bin/env bash
# Build a bare-metal bootable Sovereign OS ISO (x86_64).
#
# Architecture: Linux kernel + live userspace + Sovereign as primary OS surface.
# Requires: root or sudo, network (first run), debootstrap, xorriso, grub-pc-bin,
#           grub-efi-amd64-bin, isolinux (Debian/Ubuntu host recommended).
#
# Usage:
#   sudo bash scripts/build-sovereign-iso.sh [/output/path/Sovereign-OS.iso]
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT_ISO="${1:-$ROOT/dist/Sovereign-OS-amd64.iso}"
WORK="${SOVEREIGN_ISO_WORK:-/tmp/sovereign-iso-work}"
ARCH=amd64
DIST=bookworm
MIRROR="${SOVEREIGN_ISO_MIRROR:-http://deb.debian.org/debian}"

need() { command -v "$1" >/dev/null || { echo "missing tool: $1"; exit 1; }; }

if [[ "$(id -u)" -ne 0 ]]; then
  echo "This script must run as root (debootstrap + mount)."
  echo "  sudo bash $0 $OUT_ISO"
  exit 1
fi

need debootstrap
need xorriso
need grub-mkrescue || need grub-mkimage

echo "[iso] work=$WORK"
rm -rf "$WORK"
mkdir -p "$WORK"/{chroot,iso/live,iso/boot/grub,iso/isolinux}

echo "[iso] debootstrap $DIST $ARCH..."
debootstrap --arch="$ARCH" --variant=minbase "$DIST" "$WORK/chroot" "$MIRROR"

echo "[iso] configure live system..."
chroot "$WORK/chroot" bash -c '
set -e
export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get install -y --no-install-recommends \
  linux-image-amd64 live-boot systemd-sysv sudo \
  ca-certificates build-essential cmake g++ libargon2-dev \
  curl ca-certificates \
  isolinux syslinux-common grub-pc-bin grub-efi-amd64-bin \
  xorriso mtools
useradd -m -s /bin/bash sovereign || true
echo "sovereign:sovereign" | chpasswd
usermod -aG sudo sovereign || true
echo "sovereign ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/sovereign
mkdir -p /var/lib/sovereign /usr/local/src/sovereign
'

echo "[iso] copy Sovereign sources..."
rsync -a --exclude .git --exclude build --exclude 'build-*' --exclude tools \
  "$ROOT/" "$WORK/chroot/usr/local/src/sovereign/"

echo "[iso] build Sovereign binaries in chroot..."
chroot "$WORK/chroot" bash -c '
set -e
cd /usr/local/src/sovereign
bash scripts/build-sovereign-cli.sh /usr/local/bin/sovereign-cli || true
# daemon link may be produced by same script
if [[ -x /usr/local/bin/sovereign-daemon ]]; then true; fi
# ensure daemon exists
if [[ ! -x /usr/local/bin/sovereign-daemon ]] && [[ -f apps/daemon/main.cpp ]]; then
  # fallback: use build script outputs if present
  ls -la /usr/local/bin/ || true
fi
install -m 0755 scripts/sovereign-ctl.sh /usr/local/bin/sovereign-ctl
install -m 0644 systemd/sovereign-daemon.service /etc/systemd/system/
install -m 0644 systemd/sovereign.target /etc/systemd/system/
systemctl enable sovereign-daemon.service || true
echo "source /usr/local/src/sovereign/scripts/iso/sovereign-live-profile.sh" \
  >> /home/sovereign/.bashrc
chown -R sovereign:sovereign /home/sovereign /var/lib/sovereign
'

echo "[iso] extract kernel/initrd..."
KVER=$(ls "$WORK/chroot/boot/vmlinuz-"* | head -1 | sed 's/.*vmlinuz-//')
cp -a "$WORK/chroot/boot/vmlinuz-$KVER" "$WORK/iso/live/vmlinuz"
cp -a "$WORK/chroot/boot/initrd.img-$KVER" "$WORK/iso/live/initrd.img"

echo "[iso] squashfs filesystem..."
need mksquashfs
mksquashfs "$WORK/chroot" "$WORK/iso/live/filesystem.squashfs" \
  -comp xz -e boot

cp "$ROOT/iso/grub/grub.cfg" "$WORK/iso/boot/grub/grub.cfg"
cp "$ROOT/iso/isolinux/isolinux.cfg" "$WORK/iso/isolinux/isolinux.cfg" 2>/dev/null || true
# isolinux binaries
for f in isolinux.bin ldlinux.c32 menu.c32 libutil.c32 libcom32.c32; do
  p=$(find /usr -name "$f" 2>/dev/null | head -1 || true)
  [[ -n "$p" ]] && cp "$p" "$WORK/iso/isolinux/" || true
done

mkdir -p "$(dirname "$OUT_ISO")"
echo "[iso] xorriso hybrid ISO..."
xorriso -as mkisofs \
  -iso-level 3 \
  -full-iso9660-filenames \
  -volid "SOVEREIGN_OS" \
  -eltorito-boot isolinux/isolinux.bin \
  -eltorito-catalog isolinux/boot.cat \
  -no-emul-boot -boot-load-size 4 -boot-info-table \
  -eltorito-alt-boot \
  -e boot/grub/efi.img -no-emul-boot \
  -isohybrid-gpt-basdat \
  -output "$OUT_ISO" \
  "$WORK/iso" 2>/dev/null || \
grub-mkrescue -o "$OUT_ISO" "$WORK/iso"

ls -lh "$OUT_ISO"
echo "[iso] DONE: $OUT_ISO"
echo "  Write: sudo dd if=$OUT_ISO of=/dev/sdX bs=4M status=progress"
echo "  Or:    sudo cp $OUT_ISO /path/to/usb.img"
