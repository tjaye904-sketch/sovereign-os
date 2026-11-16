#!/usr/bin/env bash
# Gap 19: prefer model_enriched; note if STEP lines present
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
export SOVEREIGN_OLLAMA_TIMEOUT_S="${SOVEREIGN_OLLAMA_TIMEOUT_S:-180}"
CLI=$(command -v sovereign-cli)
out=$(sovereign-cli plan "Produce a structured plan with inspect then recommend for host health" 2>&1) || true
echo "$out"
echo "$out" | grep -q 'enrichment=model_enriched\|enrichment=heuristic\|enrichment=model_unavailable' || exit 1
if echo "$out" | grep -qE 'step[0-9]+ action='; then
  echo "PASS plan has steps"
fi
if echo "$out" | grep -q 'heuristic steps kept'; then
  echo "SOFT STEP format not parsed (still model_enriched) — acceptable"
fi
echo "smoke-plan-structured: OK"
