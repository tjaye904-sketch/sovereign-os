#!/usr/bin/env bash
# Sovereign OS — boot entrypoint (cognitive OS runtime, not a bare-metal ISO)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"

echo "=== Sovereign OS boot ==="
echo "root=$ROOT"

if [[ ! -x "$ROOT/build/sovereign-cli" ]]; then
  echo "[boot] building..."
  bash "$ROOT/scripts/build-sovereign-cli.sh" "$ROOT/build/sovereign-cli"
fi

export SOVEREIGN_DATA_ROOT="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}"
export OLLAMA_HOST="${OLLAMA_HOST:-http://127.0.0.1:11434}"
export SOVEREIGN_OLLAMA_MODEL="${SOVEREIGN_OLLAMA_MODEL:-openclaw}"
export SOVEREIGN_EMBED_MODEL="${SOVEREIGN_EMBED_MODEL:-nomic-embed-text}"

mkdir -p "$SOVEREIGN_DATA_ROOT"

if [[ ! -d "$SOVEREIGN_DATA_ROOT/identity" ]]; then
  echo "[boot] first run — bootstrap required"
  echo "  bash scripts/sovereign-ctl.sh bootstrap <name> <secret>"
  echo "  or: ./build/sovereign-cli bootstrap <name> <secret>"
else
  echo "[boot] data_root=$SOVEREIGN_DATA_ROOT"
fi

bash "$ROOT/scripts/sovereign-ctl.sh" status || ./build/sovereign-cli status || true
echo "[boot] start daemon: bash scripts/sovereign-ctl.sh start 60"
echo "[boot] brief:        bash scripts/sovereign-ctl.sh brief"
echo "[boot] inspect:      ./build/sovereign-cli inspect"
echo "=== Sovereign OS ready ==="
