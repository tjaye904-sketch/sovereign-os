#!/usr/bin/env bash
# Merge merge/combined-repos into main with tests. No force-push.
set -euo pipefail
ROOT="${1:-$HOME/src/sovereign-os/sovereign-work}"
cd "$ROOT"
git status
git fetch --all 2>/dev/null || true
git checkout merge/combined-repos 2>/dev/null || git checkout -b merge/combined-repos
ctest --test-dir build --output-on-failure || {
  echo "Refuse merge: tests failing on combined branch" >&2
  exit 1
}
git checkout main
git merge merge/combined-repos -m "Merge combined-repos: cognitive + host finish path"
cmake --build build -j"$(nproc)" || true
ctest --test-dir build --output-on-failure
echo "[merge] DONE on main: $(git rev-parse --short HEAD)"
