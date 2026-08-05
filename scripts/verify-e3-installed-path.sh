#!/usr/bin/env bash
# OS 2.0 E.3 — verify installed-system path artifacts exist (no host disk write)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"

ok=1
need() {
  if [[ ! -e "$1" ]]; then
    echo "MISSING: $1"
    ok=0
  else
    echo "OK: $1"
  fi
}

need "$ROOT/nix/modules/sovereign-system.nix"
need "$ROOT/scripts/iso/sovereign-first-boot.sh"
need "$ROOT/scripts/build-sovereign-vm.sh"
need "$ROOT/scripts/run-sovereign-vm.sh"
need "$ROOT/scripts/iso/sovereign-vm-boot-install.sh"

# Flake exposes system + optional vm package
if grep -q 'sovereign-system' "$ROOT/nix/flake.nix"; then
  echo "OK: flake has sovereign-system"
else
  echo "MISSING: sovereign-system in flake"
  ok=0
fi
if grep -q 'sovereign-vm\|system.build.vm' "$ROOT/nix/flake.nix"; then
  echo "OK: flake has VM build path"
else
  echo "MISSING: sovereign-vm in flake"
  ok=0
fi

# First-boot helper is non-interactive only with explicit env
if grep -q 'SOVEREIGN_BOOTSTRAP_OWNER' "$ROOT/scripts/iso/sovereign-first-boot.sh"; then
  echo "OK: first-boot gated auto-bootstrap"
else
  echo "MISSING: first-boot env gate"
  ok=0
fi

# Host install still gated
if grep -q 'SOVEREIGN_INSTALL_ALLOW_HOST' "$ROOT/scripts/iso/sovereign-install.sh"; then
  echo "OK: host install still gated"
else
  echo "MISSING: host install gate"
  ok=0
fi

[[ "$ok" -eq 1 ]] || exit 1
echo "verify-e3-installed-path: OK"
