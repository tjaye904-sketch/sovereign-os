#!/usr/bin/env bash
# F.1 — Secure Boot docs + read-only host probes (never enrolls keys)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DOC="$ROOT/docs/os2.0/SECURE_BOOT.md"

[[ -f "$DOC" ]] || { echo "missing SECURE_BOOT.md"; exit 1; }
grep -q 'Secure Boot' "$DOC"
grep -q 'Measured boot' "$DOC"
grep -q 'mokutil' "$DOC"
grep -q 'E.4' "$DOC"

echo "--- read-only probes ---"
if [[ -d /sys/firmware/efi ]]; then
  echo "firmware=UEFI"
else
  echo "firmware=no-efi-sysfs (BIOS or container)"
fi

if command -v mokutil >/dev/null 2>&1; then
  mokutil --sb-state 2>/dev/null || echo "sb-state=unavailable"
else
  echo "mokutil=not_installed (optional: sudo apt install mokutil)"
fi

if [[ -e /dev/tpm0 ]] || [[ -e /dev/tpmrm0 ]]; then
  echo "tpm=present"
else
  echo "tpm=not_visible"
fi

echo "verify-secure-boot-docs: OK"
