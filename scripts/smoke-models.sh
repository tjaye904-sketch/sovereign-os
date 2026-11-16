#!/usr/bin/env bash
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
sovereign-cli models 2>/dev/null | head -40 || true
echo "env model=${SOVEREIGN_OLLAMA_MODEL:-} embed=${SOVEREIGN_EMBED_MODEL:-}"
curl -sf http://127.0.0.1:11434/api/tags | head -c 400; echo
echo "smoke-models: OK"
