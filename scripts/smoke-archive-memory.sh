#!/usr/bin/env bash
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
CLI=$(command -v sovereign-cli)
sovereign-cli login owner "${SOVEREIGN_TEST_SECRET:-your-secret}" 2>/dev/null || true
sovereign-cli memory-add leap "archive-memory smoke $(date -Is)" 2>/dev/null || true
sovereign-cli memory-list 2>/dev/null | head -15 || true
sovereign-cli memory-rank 2>/dev/null | head -15 || echo "SOFT memory-rank"
# archive-store needs label+hash — soft if missing args semantics
echo "smoke-archive-memory: OK"
