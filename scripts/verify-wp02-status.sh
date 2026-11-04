#!/usr/bin/env bash
# WP-02 smoke: capability status must distinguish listening vs ready
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
ROOT="${1:-${HOME}/src/sovereign-os/sovereign-work}"
PY="${ROOT}/scripts/sovereign-capability-status.py"
[[ -f "$PY" ]] || PY="${HOME}/.local/bin/sovereign-capability-status"

echo "=== WP-02 verify ==="
if [[ ! -f "$PY" ]]; then
  echo "FAIL missing $PY"
  exit 1
fi

python3 "$PY" > /tmp/wp02-status.json
python3 -m json.tool < /tmp/wp02-status.json | head -50

python3 - << 'PY'
import json
st = json.load(open("/tmp/wp02-status.json"))
assert st.get("ok") is True
assert "computer_use" in st and "ready" in st["computer_use"]
assert "sys" in st and "ready" in st["sys"]
assert "operators_ready" in st
assert "reasons_not_ready" in st
cu = st["computer_use"]
disp = st.get("display") or {}
if cu.get("listening") and not disp.get("graphical_session_likely"):
    assert cu.get("ready") is False, "CU must not be ready without display"
    print("PASS truthful: CU listening without display => ready false")
else:
    print("PASS schema + ready logic")
print("operators_ready=", st.get("operators_ready"))
print("cu.ready=", cu.get("ready"), "sys.ready=", st["sys"].get("ready"))
print("reasons=", st.get("reasons_not_ready"))
PY

if curl -sf http://127.0.0.1:8766/api/capability >/dev/null 2>&1; then
  echo "PASS hub /api/capability"
else
  echo "WARN hub /api/capability not available (restart hub after patch)"
fi

echo "WP-02 verify: OK"
