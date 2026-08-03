#!/usr/bin/env bash
# Milestone B.2 — build NixOS QEMU VM for sovereign-system (no host disk writes)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if ! command -v nix >/dev/null 2>&1; then
  echo "nix required" >&2
  exit 1
fi

mkdir -p dist
echo "[vm] building nixosConfigurations.sovereign-system → system.build.vm"
echo "[vm] this can take a long time on first run..."

# Use flake in this directory
nix build "path:${ROOT}/nix#sovereign-vm" -L --out-link dist/sovereign-vm-result

echo "[vm] result: dist/sovereign-vm-result"
ls -la dist/sovereign-vm-result || true
# The result is typically a script that launches qemu
if [[ -x dist/sovereign-vm-result/bin/run-*-vm ]]; then
  ls dist/sovereign-vm-result/bin/
elif [[ -d dist/sovereign-vm-result ]]; then
  find dist/sovereign-vm-result -maxdepth 3 -type f | head -20
fi
echo "[vm] DONE — run via: bash scripts/run-sovereign-vm.sh"
