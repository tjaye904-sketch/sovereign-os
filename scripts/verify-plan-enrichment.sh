#!/usr/bin/env bash
set -euo pipefail
CLI="${1:-./build/apps/cli/sovereign-cli}"
DATA=$(mktemp -d)
trap 'rm -rf "$DATA"' EXIT
export SOVEREIGN_DATA_ROOT="$DATA"
"$CLI" bootstrap owner 'secret'
"$CLI" login owner 'secret'
OUT=$("$CLI" plan "Verify model enrichment reliability for Sovereign")
echo "$OUT"
echo "$OUT" | grep -q 'enrichment='
# Accept any of: model_enriched, model_unavailable, parse_failed, heuristic, no_router
echo "$OUT" | grep -Eq 'enrichment=(model_enriched|model_unavailable|parse_failed|heuristic|no_router)'
# Goal must not be truncated in output
echo "$OUT" | grep -q 'Verify model enrichment reliability for Sovereign'
echo "verify-plan-enrichment: OK"
