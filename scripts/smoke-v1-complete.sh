#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CLI="${1:-$ROOT/build/sovereign-cli}"
[[ -x "$CLI" ]] || bash "$ROOT/scripts/build-sovereign-cli.sh" "$CLI"
DATA=$(mktemp -d)
export SOVEREIGN_DATA_ROOT="$DATA"
export SOVEREIGN_OLLAMA_MODEL=openclaw
"$CLI" status | grep -q surface=cli-only
"$CLI" status | grep -q sync=peer-bundle
"$CLI" status | grep -q hermes=separate
"$CLI" bootstrap owner 'v1-secret'
"$CLI" login owner 'v1-secret'
"$CLI" workspace-create main development
"$CLI" memory-add note 'v1 complete'
"$CLI" memory-list note | grep -q 'v1 complete'
OP=$("$CLI" operator-create op1 'op-secret' | sed -n 's/operator_id=//p')
"$CLI" roles "$OP" | grep -q operator
"$CLI" policy-check owner configure | grep -q allow
"$CLI" policy-check readonly deploy | grep -q deny
DEC=$("$CLI" recommend ship 'v1' | sed -n 's/governance_decision_id=//p')
"$CLI" pending-decisions | grep -q "$DEC"
"$CLI" reject "$DEC" 'not-yet'
DEC2=$("$CLI" recommend ship 'v1-ok' | sed -n 's/governance_decision_id=//p')
"$CLI" approve "$DEC2" 'go'
"$CLI" execute ship "$DEC2" | grep -q executed
"$CLI" archive-store build abcdef
"$CLI" sync-export folder /tmp/v1-bundle
OWNER=$("$CLI" login owner 'v1-secret' | sed -n 's/session_id=//p' || true)
# re-login after previous logout at end — insert operator before logout
true
"$CLI" audit-list identity | grep -q bootstrap
"$CLI" logout | grep -q logged_out
"$CLI" status
rm -rf "$DATA"
echo "smoke-v1-complete: OK"
