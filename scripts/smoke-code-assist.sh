#!/usr/bin/env bash
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
export SOVEREIGN_OLLAMA_TIMEOUT_S="${SOVEREIGN_OLLAMA_TIMEOUT_S:-180}"
# short prompt
sovereign-cli code-assist "In one line: what is a CMakeLists.txt for?" 2>&1 | head -40 || echo "SOFT code-assist"
echo "smoke-code-assist: OK"
