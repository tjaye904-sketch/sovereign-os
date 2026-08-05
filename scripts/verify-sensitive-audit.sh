#!/usr/bin/env bash
set -euo pipefail
CLI="${1:-./build/apps/cli/sovereign-cli}"
DATA=$(mktemp -d)
trap 'rm -rf "$DATA"' EXIT
export SOVEREIGN_DATA_ROOT="$DATA"
"$CLI" bootstrap owner 'secret'
"$CLI" login owner 'secret'
"$CLI" policy-rule-set read operator allow
"$CLI" plan "audit coverage goal" || true
"$CLI" recommend mature "audit test"
# get pending id
PID=$("$CLI" pending-decisions | head -1 | awk '{print $1}')
if [[ -n "${PID:-}" && "$PID" != "" ]]; then
  "$CLI" reject "$PID" "a3-test-reject" || true
fi
"$CLI" logout
LIST=$("$CLI" audit-list 2>/dev/null || true)
# re-login to list audit? audit-list may not need session
"$CLI" login owner 'secret'
LIST=$("$CLI" audit-list)
echo "$LIST"
echo "$LIST" | grep -q identity
echo "$LIST" | grep -Eq 'policy|governance|cognitive|identity'
echo "verify-sensitive-audit: OK"
