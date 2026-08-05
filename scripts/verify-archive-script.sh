#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
[[ -x "$ROOT/scripts/archive-worktree.sh" ]] || chmod +x "$ROOT/scripts/archive-worktree.sh"
grep -q 'git archive' "$ROOT/scripts/archive-worktree.sh"
grep -q 'bundle' "$ROOT/scripts/archive-worktree.sh"
[[ -f "$ROOT/docs/os2.0/ARCHIVE.md" ]]
echo "verify-archive-script: OK"
