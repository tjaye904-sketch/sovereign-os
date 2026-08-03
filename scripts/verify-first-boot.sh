#!/usr/bin/env bash
# B.3 — first-boot helper on host CLI (temp data root)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CLI="${1:-$ROOT/build/apps/cli/sovereign-cli}"
FB="$ROOT/scripts/iso/sovereign-first-boot.sh"
chmod +x "$FB"

DATA=$(mktemp -d)
trap 'rm -rf "$DATA"' EXIT
export SOVEREIGN_DATA_ROOT="$DATA"
# Put CLI on PATH for helper
export PATH="$(dirname "$CLI"):$PATH"
ln -sf "$(realpath "$CLI")" "$DATA/sovereign-cli" 2>/dev/null || true
# Helper looks for sovereign-cli on PATH
PATH="$(dirname "$(realpath "$CLI")"):$PATH" bash "$FB" | tee /tmp/fb1.out
grep -q "first boot\|first-boot\|Manual\|No auto-bootstrap\|bootstrap" /tmp/fb1.out

# Auto path
export SOVEREIGN_BOOTSTRAP_OWNER=owner
export SOVEREIGN_BOOTSTRAP_SECRET=secret
PATH="$(dirname "$(realpath "$CLI")"):$PATH" bash "$FB" | tee /tmp/fb2.out
test -f "$DATA/.first-boot-complete"
# Second run is idempotent
PATH="$(dirname "$(realpath "$CLI")"):$PATH" bash "$FB" | tee /tmp/fb3.out
grep -q "already complete" /tmp/fb3.out

echo "verify-first-boot: OK"
