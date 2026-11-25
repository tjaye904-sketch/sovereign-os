#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

# Gate 1: Required NixOS lab artifacts exist
for f in flake.nix flake.lock nix/modules/sovereign-node.nix nix/modules/sovereign-system.nix nix/modules/sovereign-iso.nix nix/modules/sovereign-vm-minimal.nix nix/packages/sovereign-cli.nix nix/packages/sovereign-daemon.nix; do
  [ -f "$f" ] || { echo "MISSING: $f"; exit 1; }
done

# Gate 2: Headless node module enforces hardening & non-X11 profile
grep -q 'services.xserver.enable = false;' nix/modules/sovereign-node.nix
grep -q 'NoNewPrivileges = true;' nix/modules/sovereign-node.nix
grep -q 'ProtectSystem = "strict";' nix/modules/sovereign-node.nix
grep -q 'SOVEREIGN_PROFILE = "nixos-sovereign";' nix/modules/sovereign-node.nix
grep -q 'nixosConfigurations.sovereign-node' flake.nix

# Gate 3: If nix binary is installed, evaluate flake outputs without touching host disk
if command -v nix >/dev/null 2>&1; then
  nix flake metadata --no-write-lock-file . >/dev/null
  nix eval --raw .#nixosConfigurations.sovereign-node.config.networking.hostName | grep -q "sovereign-node"
fi

echo "NIXOS_LAB_GATES_OK"
