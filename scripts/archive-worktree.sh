#!/usr/bin/env bash
# F.4 — archive Sovereign worktree for backup
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT_DIR="$HOME/Downloads"
INCLUDE_ISO=0
INCLUDE_GIT=1

while [[ $# -gt 0 ]]; do
  case "$1" in
    --out) OUT_DIR="${2:-}"; shift 2 ;;
    --with-iso) INCLUDE_ISO=1; shift ;;
    --no-git) INCLUDE_GIT=0; shift ;;
    -h|--help)
      echo "Usage: $0 [--out DIR] [--with-iso] [--no-git]"
      exit 0
      ;;
    *) echo "unknown: $1" >&2; exit 1 ;;
  esac
done

mkdir -p "$OUT_DIR"
STAMP="$(date -u +%Y%m%dT%H%M%SZ)"
SHORT="$(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null || echo nogit)"
OUT="$OUT_DIR/Sovereign-worktree-${SHORT}-${STAMP}.zip"

cd "$ROOT"
if [[ ! -d .git ]]; then
  echo "not a git repo; packing filtered tree"
  zip -rq "$OUT" . \
    -x 'build/*' -x 'build-*/' -x 'result' -x 'result-*' \
    -x '*.o' -x '*.a' -x 'dist/*.iso' -x '.direnv/*'
else
  # Source tree as committed + HEAD description
  git archive --format=zip --prefix=sovereign-work/ -o "$OUT" HEAD
  # Append .git for full history (optional)
  if [[ "$INCLUDE_GIT" -eq 1 ]]; then
    TMP=$(mktemp -d)
    # bundle is smaller/safer than raw .git for transfer
    git bundle create "$TMP/sovereign.bundle" --all
    (
      cd "$TMP"
      zip -q "$OUT" sovereign.bundle
    )
    rm -rf "$TMP"
  fi
  if [[ "$INCLUDE_ISO" -eq 1 && -f dist/Sovereign-OS-nix-amd64.iso ]]; then
    zip -q "$OUT" dist/Sovereign-OS-nix-amd64.iso
  fi
fi

# Manifest
{
  echo "archive=$OUT"
  echo "created_utc=$STAMP"
  echo "head=$SHORT"
  git -C "$ROOT" log -1 --oneline 2>/dev/null || true
  echo "tests=run ctest on extract"
} > "$OUT_DIR/Sovereign-worktree-${SHORT}-${STAMP}.txt"

ls -lh "$OUT"
cat "$OUT_DIR/Sovereign-worktree-${SHORT}-${STAMP}.txt"
echo "[archive] DONE"
