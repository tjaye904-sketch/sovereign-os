#!/usr/bin/env bash
# F.3 — package channel / flake pin verification
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../" && pwd)"
NIX="$ROOT/nix"
ok=1

need() {
  if [[ ! -f "$1" ]]; then
    echo "MISSING: $1"
    ok=0
  else
    echo "OK: $1"
  fi
}

need "$NIX/flake.nix"
need "$NIX/flake.lock"
need "$ROOT/docs/os2.0/PACKAGE_CHANNEL.md"

if grep -q 'nixos-24.11' "$NIX/flake.nix"; then
  echo "OK: flake.nix pins nixos-24.11"
else
  echo "WARN: flake.nix does not mention nixos-24.11"
fi

if grep -q 'nixpkgs' "$NIX/flake.lock"; then
  echo "OK: flake.lock contains nixpkgs node"
else
  echo "MISSING: nixpkgs in flake.lock"
  ok=0
fi

# locked rev present
if grep -q '"rev"' "$NIX/flake.lock"; then
  echo "OK: flake.lock has rev pin(s)"
  # print first rev for evidence
  grep -m1 '"rev"' "$NIX/flake.lock" || true
else
  echo "MISSING: rev in flake.lock"
  ok=0
fi

for out in iso sovereign-cli sovereign-daemon sovereign-vm; do
  if grep -q "$out" "$NIX/flake.nix"; then
    echo "OK: output $out declared"
  else
    echo "MISSING: output $out"
    ok=0
  fi
done

[[ "$ok" -eq 1 ]] || exit 1
echo "verify-package-channel: OK"
