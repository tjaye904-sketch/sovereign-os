#!/usr/bin/env bash
# Offline product smoke (no Ollama required)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CLI="${1:-$ROOT/build/sovereign-cli}"
if [[ ! -x "$CLI" ]]; then
  bash "$ROOT/scripts/build-sovereign-cli.sh" "$CLI"
fi
DATA=$(mktemp -d)
export SOVEREIGN_DATA_ROOT="$DATA"
export SOVEREIGN_OLLAMA_MODEL=openclaw
"$CLI" status
"$CLI" bootstrap owner 'offline-secret'
"$CLI" login owner 'offline-secret'
"$CLI" workspace-create main development
"$CLI" memory-add smoke 'offline ok'
DEC=$("$CLI" recommend deploy 'offline ship' | sed -n 's/governance_decision_id=//p')
"$CLI" approve "$DEC" 'ok'
"$CLI" execute deploy "$DEC"
"$CLI" status
rm -rf "$DATA"
echo "smoke-offline: OK"
