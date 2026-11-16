#!/usr/bin/env bash
# Gap 11: evolution path present in build + optional CLI memory note
set -euo pipefail
ROOT="${1:-$HOME/src/sovereign-os/sovereign-work}"
export PATH="$HOME/.local/bin:$PATH"
ctest --test-dir "$ROOT/build" -R 'evolution' --output-on-failure
CLI=$(command -v sovereign-cli || true)
[[ -x "$CLI" ]] || CLI="$ROOT/build/apps/cli/sovereign-cli"
if [[ -x "$CLI" ]]; then
  "$CLI" memory-add evolution "operator path verified $(date -Is)" 2>/dev/null || true
fi
echo "verify-evolution-path: OK"
