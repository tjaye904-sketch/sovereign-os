#!/usr/bin/env bash
# WP-14: deny-by-default — keep only a curated allow seed
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
ROOT="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}"
POL="$ROOT/inventory/app_policy.json"
mkdir -p "$ROOT/inventory"

# Curated safe defaults for daily host operator path
ALLOW_BINS=(
  gnome-system-monitor
  cosmic-files
  nautilus
  gnome-terminal
  cosmic-term
  xdg-open
  chromium
  chromium-browser
  google-chrome
  firefox
  gedit
  libreoffice
  libreoffice-writer
)

python3 - << 'PY'
import json, os
from pathlib import Path
root = Path(os.environ.get("SOVEREIGN_DATA_ROOT", Path.home() / ".local/share/sovereign"))
pol_p = root / "inventory" / "app_policy.json"
inv_p = root / "inventory" / "apps.json"
allow_bins = set("""gnome-system-monitor cosmic-files nautilus gnome-terminal cosmic-term xdg-open chromium chromium-browser google-chrome firefox gedit libreoffice libreoffice-writer""".split())
apps = []
if inv_p.is_file():
    apps = json.loads(inv_p.read_text()).get("apps", [])
pol = {"version": 2, "mode": "deny_by_default", "apps": {}}
# map id/bin
for a in apps:
    aid = a.get("id") or ""
    b = (a.get("bin") or "").lower()
    name = a.get("name") or aid
    allow = b in allow_bins or aid in allow_bins
    pol["apps"][aid] = {
        "allow": allow,
        "bin": a.get("bin") or aid,
        "name": name,
        "policy": "allow" if allow else "deny",
    }
# ensure monitor keys exist
for key in ("gnome-system-monitor", "gnome-system-monitor-kde"):
    if key not in pol["apps"]:
        pol["apps"][key] = {"allow": True, "bin": "gnome-system-monitor", "name": "System Monitor", "policy": "allow"}
pol_p.write_text(json.dumps(pol, indent=2))
allowed = sum(1 for v in pol["apps"].values() if v.get("allow"))
print(f"WP-14 hardened: {allowed}/{len(pol['apps'])} allowed (deny-by-default)")
PY

# re-allow critical for operator path
sovereign-inventory allow gnome-system-monitor 2>/dev/null || true
sovereign-inventory allow gnome-system-monitor-kde 2>/dev/null || true
echo "WP-14 done — review: sovereign-inventory list | head"
