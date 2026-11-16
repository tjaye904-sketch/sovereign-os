#!/usr/bin/env bash
# Raise Ollama client timeouts in env + best-effort patch of ollama_client if present.
set -euo pipefail
ROOT="${1:-$HOME/src/sovereign-os/sovereign-work}"
ENVF="$HOME/.config/sovereign/env"
mkdir -p "$HOME/.config/sovereign"
touch "$ENVF"
grep -q '^SOVEREIGN_OLLAMA_TIMEOUT_S=' "$ENVF" 2>/dev/null \
  && sed -i 's/^SOVEREIGN_OLLAMA_TIMEOUT_S=.*/SOVEREIGN_OLLAMA_TIMEOUT_S=180/' "$ENVF" \
  || echo 'SOVEREIGN_OLLAMA_TIMEOUT_S=180' >> "$ENVF"
grep -q '^SOVEREIGN_PLAN_TIMEOUT_S=' "$ENVF" 2>/dev/null \
  || echo 'SOVEREIGN_PLAN_TIMEOUT_S=180' >> "$ENVF"

# Best-effort: bump hardcoded timeouts in tree
for f in \
  "$ROOT/core/model_router/ollama_client.cpp" \
  "$ROOT/core/model_router/model_router.cpp" \
  "$ROOT/core/cognitive/cognitive_service.cpp"; do
  [[ -f "$f" ]] || continue
  # common patterns 30/60 second timeouts → 180
  sed -i -E \
    -e 's/timeout_s\s*=\s*30/timeout_s = 180/g' \
    -e 's/timeout_s\s*=\s*60/timeout_s = 180/g' \
    -e 's/TIMEOUT_MS\s*=\s*30000/TIMEOUT_MS = 180000/g' \
    -e 's/TIMEOUT_MS\s*=\s*60000/TIMEOUT_MS = 180000/g' \
    -e 's/connect_timeout[^;]*=\s*5/connect_timeout = 30/g' \
    "$f" 2>/dev/null || true
done
echo "[fix-plan] env timeouts set; source files touched if present"
echo "  Rebuild: cmake --build $ROOT/build -j\$(nproc) --target sovereign-cli"
echo "  Test:    SOVEREIGN_OLLAMA_TIMEOUT_S=180 sovereign-cli plan 'enrichment live test'"
