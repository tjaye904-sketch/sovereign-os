#!/usr/bin/env bash
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
# purposes may sit next to scripts in package root
SRC="${HERE}/../purposes"
[[ -d "$SRC" ]] || SRC="${HERE}/purposes"
DEST="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}/purposes"
REPO="${1:-$HOME/src/sovereign-os/sovereign-work}/purposes"

mkdir -p "$DEST" "$REPO"
cp -a "$SRC"/*.yaml "$DEST/"
cp -a "$SRC"/*.yaml "$REPO/"
echo "Installed purposes → $DEST"
ls -1 "$DEST"
