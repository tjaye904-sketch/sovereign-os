#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/src/sovereign-os/sovereign-work}"
OUT="$ROOT/docs/project/PROJECT_TREE.md"
{
  echo "# Sovereign PROJECT_TREE (auto)"
  echo
  echo "Generated: $(date -Is)"
  echo "Branch: $(git -C "$ROOT" branch --show-current 2>/dev/null || echo ?)"
  echo "HEAD: $(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null || echo ?)"
  echo
  echo "## Top-level"
  echo '```'
  ls -la "$ROOT" | head -60
  echo '```'
  echo
  echo "## core/"
  echo '```'
  find "$ROOT/core" -maxdepth 2 -type d 2>/dev/null | sort | head -80
  echo '```'
  echo
  echo "## apps/ scripts/ docs/os2.0/"
  echo '```'
  ls "$ROOT/apps" 2>/dev/null
  ls "$ROOT/scripts" 2>/dev/null | head -40
  ls "$ROOT/docs/os2.0" 2>/dev/null | head -40
  echo '```'
} > "$OUT"
echo "wrote $OUT"
