#!/usr/bin/env bash
# Copy the Nix run-vm backing disk into a stable path under $HOME/sov-vm.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEST="${1:-$HOME/sov-vm/sovereign-installed.qcow2}"
LINK="$ROOT/dist/sovereign-vm-result"

mkdir -p "$(dirname "$DEST")"
if [[ ! -e "$LINK" ]]; then
  echo "Build first: bash scripts/build-sovereign-vm.sh full" >&2
  exit 1
fi

# NixOS run-*-vm usually keeps state under /tmp or beside the result; also check cwd
CANDIDATES=(
  "$ROOT/sovereign-vm.qcow2"
  "$ROOT/nixos.qcow2"
  "$HOME/sovereign-vm.qcow2"
)
# Search under result and /tmp for recent qcow2 from this project
while IFS= read -r -d '' f; do
  CANDIDATES+=("$f")
done < <(find -L "$LINK" /tmp -maxdepth 3 -name '*.qcow2' -type f 2>/dev/null | head -20 | tr '\n' '\0' || true)

SRC=""
for c in "${CANDIDATES[@]}"; do
  if [[ -f "$c" ]]; then
    SRC="$c"
    break
  fi
done

if [[ -z "$SRC" ]]; then
  echo "[persist] No existing qcow2 found."
  echo "[persist] Creating empty 40G image at $DEST"
  echo "[persist] Boot once with run-sovereign-vm-ssh.sh full, shut down, re-run this script,"
  echo "          or use boot-installed-qcow2.sh after a first Nix VM run creates a disk."
  qemu-img create -f qcow2 "$DEST" 40G
  ls -lh "$DEST"
  exit 0
fi

echo "[persist] source=$SRC"
echo "[persist] dest=$DEST"
cp -a "$SRC" "$DEST"
qemu-img info "$DEST" | head -15
echo "[persist] DONE"
