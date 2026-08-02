#!/usr/bin/env bash
# Build Sovereign OS live ISO using NixOS (flake).
#
# Requirements (host):
#   - Nix with flakes enabled
#   - ~15–25 GB free disk, 8+ GB RAM recommended (16+ ideal)
#
# Usage:
#   bash scripts/build-sovereign-nix-iso.sh [output-dir]
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT_DIR="${1:-$ROOT/dist}"
NIX_DIR="$ROOT/nix"

need() { command -v "$1" >/dev/null || { echo "missing: $1 — install Nix: https://nixos.org/download"; exit 1; }; }
need nix

mkdir -p "$OUT_DIR"
cd "$NIX_DIR"

echo "[nix-iso] evaluating + building Sovereign ISO (this can take a long time)..."
# Enable flakes if needed via experimental flags
export NIX_CONFIG="${NIX_CONFIG:-}
experimental-features = nix-command flakes"

nix build ".#iso" -L --out-link "$OUT_DIR/sovereign-iso-result"

ISO_PATH=$(find "$OUT_DIR/sovereign-iso-result" -name '*.iso' 2>/dev/null | head -1 || true)
if [[ -z "${ISO_PATH}" ]]; then
  ISO_PATH=$(find -L "$OUT_DIR/sovereign-iso-result" -name '*.iso' 2>/dev/null | head -1 || true)
fi

if [[ -n "${ISO_PATH}" ]]; then
  cp -L "$ISO_PATH" "$OUT_DIR/Sovereign-OS-nix-amd64.iso"
  ls -lh "$OUT_DIR/Sovereign-OS-nix-amd64.iso"
  echo "[nix-iso] DONE: $OUT_DIR/Sovereign-OS-nix-amd64.iso"
else
  echo "[nix-iso] build finished; listing result:"
  find -L "$OUT_DIR/sovereign-iso-result" -type f | head -40
  echo "[nix-iso] copy the .iso from the result path above to $OUT_DIR/"
fi
