#!/usr/bin/env bash
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
ROOT="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}"
POL="$ROOT/inventory/app_policy.json"

echo "=== WP-07 verify ==="

if ! curl -sf http://127.0.0.1:8767/health >/dev/null; then
  echo "FAIL sys not up"
  exit 1
fi

# Use CLI allow if present
if command -v sovereign-inventory >/dev/null; then
  sovereign-inventory allow gnome-system-monitor
  sovereign-inventory deny gucharmap
else
  echo "WARN no sovereign-inventory CLI"
fi

# Show format
python3 - << PY
import json
from pathlib import Path
p = Path("$POL")
data = json.loads(p.read_text())
apps = data.get("apps", {})
gsm = apps.get("gnome-system-monitor") or apps.get("org.gnome.SystemMonitor")
# also search by bin
if not gsm:
    for k,v in apps.items():
        if isinstance(v, dict) and v.get("bin") == "gnome-system-monitor":
            gsm = v
            print("found via bin", k)
            break
print("gnome-system-monitor entry:", json.dumps(gsm))
assert gsm is not None, "missing allow entry"
if isinstance(gsm, dict):
    assert gsm.get("allow") is True or str(gsm.get("policy","")).lower()=="allow"
elif isinstance(gsm, str):
    assert gsm.lower() in ("allow", "allowed")
print("PASS policy format for allow")
PY

OUT=$(sovereign-sys app_open --args '{"target":"gucharmap"}' 2>/dev/null || true)
echo "$OUT" | grep -q '"ok": false' || { echo "FAIL gucharmap should deny: $OUT"; exit 1; }
echo "PASS CLI deny → sys deny"

OUT2=$(sovereign-sys app_open --args '{"target":"gnome-system-monitor"}' 2>/dev/null || true)
echo "$OUT2"
if echo "$OUT2" | grep -qi 'inventory deny'; then
  echo "FAIL allow via CLI still inventory-denied"
  exit 1
fi
echo "PASS CLI allow → sys not inventory-deny"
echo "WP-07 verify: OK"
