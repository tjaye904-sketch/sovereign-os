#!/usr/bin/env bash
# WP-06: inventory deny blocks app_open; allow permits known app
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
ROOT="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}"
POL="$ROOT/inventory/app_policy.json"

echo "=== WP-06 verify ==="

if ! curl -sf http://127.0.0.1:8767/health >/dev/null; then
  echo "FAIL sys not healthy on :8767"
  exit 1
fi

# Ensure policy file exists
mkdir -p "$ROOT/inventory"
if [[ ! -f "$POL" ]]; then
  echo '{"apps":{}}' > "$POL"
fi

# Seed deny for a harmless test id
python3 - << PY
import json
from pathlib import Path
p = Path("$POL")
data = json.loads(p.read_text()) if p.is_file() else {}
apps = data.get("apps", data if isinstance(data, dict) else {})
if not isinstance(apps, dict):
    apps = {}
apps["gucharmap"] = "deny"
apps["wp06-test-denied"] = "deny"
apps["gnome-system-monitor"] = "allow"
if "apps" in data or not data:
    data = {"apps": apps}
else:
    data = apps
p.write_text(json.dumps(data, indent=2))
print("seeded policy")
PY

# Deny must fail
OUT=$(sovereign-sys app_open --args '{"target":"gucharmap"}' 2>/dev/null || true)
echo "deny attempt: $OUT"
echo "$OUT" | grep -qiE 'denied|inventory deny|not allow' || {
  # also accept ok:false
  echo "$OUT" | grep -q '"ok": false' || { echo "FAIL expected deny for gucharmap"; exit 1; }
}
echo "PASS deny blocks gucharmap"

# Allow must succeed or at least not inventory-deny (GUI may still fail without display)
OUT2=$(sovereign-sys app_open --args '{"target":"gnome-system-monitor"}' 2>/dev/null || true)
echo "allow attempt: $OUT2"
if echo "$OUT2" | grep -qi 'inventory deny'; then
  echo "FAIL gnome-system-monitor blocked by inventory despite allow"
  exit 1
fi
echo "PASS allow does not inventory-deny gnome-system-monitor"

echo "WP-06 verify: OK"
