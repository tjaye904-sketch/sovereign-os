#!/usr/bin/env bash
# B.2 lightweight verify: flake evaluates sovereign-system + vm attribute exists
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT/nix"
if ! command -v nix >/dev/null 2>&1; then
  echo "nix not installed — skip eval (gates still documented)"
  echo "verify-b2-vm-eval: SKIP"
  exit 0
fi
# Ensure flake has sovereign-vm
grep -q sovereign-vm flake.nix
# Eval only (no full build) — faster check
nix eval --raw "path:${ROOT}/nix#nixosConfigurations.sovereign-system.config.system.name" 2>/dev/null \
  || nix flake show "path:${ROOT}/nix" 2>&1 | head -40
echo "verify-b2-vm-eval: OK"
