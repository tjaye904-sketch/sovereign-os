#!/usr/bin/env bash
# Boot-test a Sovereign ISO in QEMU (non-interactive smoke).
#
# Usage:
#   bash scripts/test-sovereign-iso-boot.sh [path-to.iso]
#
# Passes if QEMU starts, kernel begins boot, and we see Sovereign/NixOS
# markers in serial log within timeout (default 180s).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ISO="${1:-$ROOT/dist/Sovereign-OS-nix-amd64.iso}"
TIMEOUT="${SOVEREIGN_BOOT_TIMEOUT:-180}"
LOG="${SOVEREIGN_BOOT_LOG:-/tmp/sovereign-iso-boot.log}"
MEM="${SOVEREIGN_QEMU_MEM:-2048}"

need() { command -v "$1" >/dev/null || { echo "missing: $1"; exit 1; }; }
need qemu-system-x86_64

if [[ ! -f "$ISO" ]]; then
  echo "ISO not found: $ISO"
  echo "Build first: bash scripts/build-sovereign-nix-iso.sh"
  exit 1
fi

rm -f "$LOG"
echo "[boot-test] ISO=$ISO mem=${MEM}M timeout=${TIMEOUT}s"
echo "[boot-test] serial log: $LOG"

# -nographic + serial: capture boot console
# -no-reboot: exit on panic/reboot instead of looping
timeout "$TIMEOUT" qemu-system-x86_64 \
  -machine q35 \
  -m "$MEM" \
  -smp 2 \
  -cdrom "$ISO" \
  -boot d \
  -nographic \
  -serial mon:stdio \
  -no-reboot \
  -device virtio-net-pci,netdev=n0 \
  -netdev user,id=n0 \
  >"$LOG" 2>&1 &
QEMU_PID=$!

# Poll log for success markers
PASS=0
for i in $(seq 1 "$TIMEOUT"); do
  if ! kill -0 "$QEMU_PID" 2>/dev/null; then
    break
  fi
  if grep -qiE 'Sovereign OS|welcome to NixOS|login:|sovereign-cli|multi-user.target' "$LOG" 2>/dev/null; then
    PASS=1
    break
  fi
  # Kernel clearly running
  if grep -qiE 'Linux version|Kernel command line' "$LOG" 2>/dev/null; then
    # keep waiting for userspace markers
    true
  fi
  sleep 1
done

kill "$QEMU_PID" 2>/dev/null || true
wait "$QEMU_PID" 2>/dev/null || true

echo "----- boot log (tail) -----"
tail -80 "$LOG" || true
echo "----- end log -----"

if [[ "$PASS" -eq 1 ]]; then
  echo "[boot-test] PASS — boot markers found"
  exit 0
fi

if grep -qiE 'Kernel command line|Linux version' "$LOG" 2>/dev/null; then
  echo "[boot-test] PARTIAL — kernel started; userspace markers not seen in ${TIMEOUT}s"
  echo "  Increase SOVEREIGN_BOOT_TIMEOUT or SOVEREIGN_QEMU_MEM and retry"
  exit 2
fi

echo "[boot-test] FAIL — no boot markers"
exit 1
