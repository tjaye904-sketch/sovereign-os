#!/usr/bin/env bash
# Milestone B.3 — first-boot identity helper (live ISO / installed system / VM)
# Never invents a password. Optional non-interactive bootstrap only if env set.
set -euo pipefail

export SOVEREIGN_DATA_ROOT="${SOVEREIGN_DATA_ROOT:-/var/lib/sovereign}"
mkdir -p "$SOVEREIGN_DATA_ROOT"

CLI="$(command -v sovereign-cli || true)"
if [[ -z "$CLI" ]]; then
  # Host build tree fallback
  for c in ./build/apps/cli/sovereign-cli ../build/apps/cli/sovereign-cli; do
    if [[ -x "$c" ]]; then CLI="$(realpath "$c")"; break; fi
  done
fi

marker="$SOVEREIGN_DATA_ROOT/.first-boot-complete"

if [[ -f "$marker" ]]; then
  echo "[first-boot] already complete ($marker)"
  if [[ -n "$CLI" ]]; then
    "$CLI" status || true
  fi
  exit 0
fi

echo "=== Sovereign first boot ==="
echo "data_root=$SOVEREIGN_DATA_ROOT"

if [[ -n "${SOVEREIGN_BOOTSTRAP_OWNER:-}" && -n "${SOVEREIGN_BOOTSTRAP_SECRET:-}" ]]; then
  if [[ -z "$CLI" ]]; then
    echo "[first-boot] sovereign-cli not found; cannot auto-bootstrap" >&2
    exit 1
  fi
  echo "[first-boot] non-interactive bootstrap owner=${SOVEREIGN_BOOTSTRAP_OWNER}"
  if ! "$CLI" bootstrap "$SOVEREIGN_BOOTSTRAP_OWNER" "$SOVEREIGN_BOOTSTRAP_SECRET"; then
    # already bootstrapped is acceptable
    echo "[first-boot] bootstrap returned non-zero (may already exist)"
  fi
  "$CLI" login "$SOVEREIGN_BOOTSTRAP_OWNER" "$SOVEREIGN_BOOTSTRAP_SECRET"
  "$CLI" status || true
  date -Is > "$marker"
  echo "[first-boot] complete (auto)"
  exit 0
fi

# Interactive guidance only
echo "No auto-bootstrap (set SOVEREIGN_BOOTSTRAP_OWNER + SOVEREIGN_BOOTSTRAP_SECRET to enable)."
echo "Manual:"
echo "  export SOVEREIGN_DATA_ROOT=$SOVEREIGN_DATA_ROOT"
echo "  sovereign-cli bootstrap <name> <secret>"
echo "  sovereign-cli login <name> <secret>"
echo "  sovereign-cli brief"
echo "  touch $marker"
if [[ -n "$CLI" ]]; then
  "$CLI" status || true
fi
exit 0
