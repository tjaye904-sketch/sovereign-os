#!/usr/bin/env bash
# First-boot helper on Sovereign live/install image
set -euo pipefail
export SOVEREIGN_DATA_ROOT="${SOVEREIGN_DATA_ROOT:-/var/lib/sovereign}"
mkdir -p "$SOVEREIGN_DATA_ROOT"

if [[ ! -d "$SOVEREIGN_DATA_ROOT/identity" ]]; then
  echo "=== Sovereign first boot ==="
  echo "No identity store found. Bootstrap an owner:"
  echo "  sovereign-cli bootstrap <name> <secret>"
  echo "  sovereign-cli login <name> <secret>"
  echo "  sovereign-cli brief"
else
  echo "Sovereign data present at $SOVEREIGN_DATA_ROOT"
  sovereign-cli status || true
fi
