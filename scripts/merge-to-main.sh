#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/src/sovereign-os/sovereign-work}"
cd "$ROOT"
git status
git checkout merge/combined-repos
ctest --test-dir build --output-on-failure
git checkout main
git merge merge/combined-repos -m "Merge branch 'merge/combined-repos' — cognitive host + maturity"
cmake --build build -j"$(nproc)" || true
ctest --test-dir build --output-on-failure
git log --oneline -5
echo "[merge] HEAD=$(git rev-parse --short HEAD) branch=$(git branch --show-current)"
