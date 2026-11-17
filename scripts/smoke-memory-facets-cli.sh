#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$HOME/src/sovereign-os/sovereign-work}"
CLI="$ROOT/build/apps/cli/sovereign-cli"
[[ -x "$CLI" ]] || CLI=$(command -v sovereign-cli)
DATA=$(mktemp -d)
export SOVEREIGN_DATA_ROOT="$DATA"
"$CLI" memory-facet-add constitutional arch "Sovereign owns federation authority"
"$CLI" memory-facet-add working task "L1.2 facets leap"
"$CLI" memory-facet-counts | grep -q 'constitutional'
"$CLI" memory-facet-list constitutional | grep -q federation
"$CLI" inspect memory 2>/dev/null | grep -q facet_counts || true
rm -rf "$DATA"
echo "smoke-memory-facets-cli: OK"
