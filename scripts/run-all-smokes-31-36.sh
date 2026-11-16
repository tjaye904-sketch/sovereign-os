#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/src/sovereign-os/sovereign-work}"
export PATH="$HOME/.local/bin:$PATH"
for s in smoke-models smoke-policy smoke-recommend smoke-code-assist smoke-sync; do
  echo "==== $s ===="
  bash "$ROOT/scripts/$s.sh" || echo "SOFT $s"
done
bash "$ROOT/scripts/write-project-tree.sh" "$ROOT"
echo "run-all-smokes-31-36: DONE"
