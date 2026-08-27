#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
TARGET="${1:-minimal}"

if ! command -v nix >/dev/null 2>&1; then
  echo "nix required" >&2
  exit 1
fi

mkdir -p dist
ATTR="sovereign-vm-minimal"
OUT="dist/sovereign-vm-minimal-result"
if [[ "$TARGET" == "full" ]]; then
  ATTR="sovereign-vm"
  OUT="dist/sovereign-vm-result"
fi

echo "[vm] building path:${ROOT}#${ATTR}"
nix build "path:${ROOT}#${ATTR}" -L --out-link "$OUT"
echo "[vm] result: $OUT"
find -L "$OUT" -maxdepth 4 -type f -name 'run-*-vm' 2>/dev/null | head
echo "[vm] DONE — bash scripts/run-sovereign-vm.sh ${TARGET}"
