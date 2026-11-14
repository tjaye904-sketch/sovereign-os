#!/usr/bin/env python3
"""Patch installed sovereign-hub-server to add /api/inventory/* routes (WP-08)."""
from __future__ import annotations

import re
import sys
from pathlib import Path

HOME = Path.home()
CANDIDATES = [
    HOME / ".local/bin/sovereign-hub-server",
    HOME / ".local/share/sovereign-hub/bin/sovereign-hub-server",
]

INVENTORY_SNIPPET = r'''
# --- WP-08 inventory API ---
def _inv_handle(path: str, body: dict | None = None):
    body = body or {}
    try:
        from sovereign_inventory import scan as inv_scan
    except Exception:
        inv_scan = None
    import json as _json
    from pathlib import Path as _P
    root = _P(__import__("os").environ.get("SOVEREIGN_DATA_ROOT", str(_P.home() / ".local/share/sovereign")))
    if path.endswith("/scan") or path == "scan":
        if inv_scan:
            p = inv_scan.scan_apps()
            return {"ok": True, "count": p.get("count"), "scanned_at": p.get("scanned_at")}
        return {"ok": False, "error": "sovereign_inventory missing"}
    if path.endswith("/set") or path == "set":
        aid = str(body.get("id") or "")
        allow = bool(body.get("allow", False))
        if inv_scan:
            return inv_scan.set_allow(aid, allow)
        return {"ok": False, "error": "sovereign_inventory missing"}
    # list
    if inv_scan:
        return inv_scan.inventory_with_policy()
    inv_p = root / "inventory" / "apps.json"
    pol_p = root / "inventory" / "app_policy.json"
    apps = _json.loads(inv_p.read_text()).get("apps", []) if inv_p.is_file() else []
    pol = _json.loads(pol_p.read_text()).get("apps", {}) if pol_p.is_file() else {}
    out = []
    for a in apps:
        meta = pol.get(a.get("id", {}), {})
        allow = bool(meta.get("allow")) if isinstance(meta, dict) else False
        out.append({**a, "allow": allow})
    return {"ok": True, "count": len(out), "allowed_count": sum(1 for x in out if x["allow"]), "apps": out}
# --- end WP-08 ---
'''


def find_server() -> Path | None:
    for c in CANDIDATES:
        if c.is_file():
            return c
    # which
    import shutil
    w = shutil.which("sovereign-hub-server")
    return Path(w) if w else None


def patch(path: Path) -> bool:
    t = path.read_text(encoding="utf-8", errors="replace")
    if "_inv_handle" in t and "/api/inventory" in t:
        print("already patched", path)
        return True
    if "_inv_handle" not in t:
        # insert before class or main handler
        if "def ensure_ready" in t:
            t = t.replace("def ensure_ready", INVENTORY_SNIPPET + "\ndef ensure_ready", 1)
        else:
            t = INVENTORY_SNIPPET + "\n" + t

    # Hook do_GET / do_POST inventory paths — common patterns
    if "api/inventory" not in t:
        # inject into a typical do_GET path parse block
        marker = 'if path.startswith("/api/'
        # simpler: append routing helper called from existing api handler
        inject_get = '''
        if path.startswith("/api/inventory"):
            self.send_json(_inv_handle(path))
            return
'''
        inject_post = '''
        if path.startswith("/api/inventory"):
            self.send_json(_inv_handle(path, body if isinstance(body, dict) else {}))
            return
'''
        # Try several patterns
        if "def do_GET" in t and "api/inventory" not in t:
            t2, n = re.subn(
                r"(def do_GET\(self\):.*?\n)",
                r"\1" + "        # WP-08\n" + inject_get,
                t,
                count=1,
                flags=re.S,
            )
            if n:
                t = t2
            else:
                # after path = urlparse
                t = t.replace(
                    "path = urlparse(self.path).path",
                    "path = urlparse(self.path).path\n        if path.startswith(\"/api/inventory\"):\n            self._send_json(_inv_handle(path)); return",
                    1,
                )
        if "def do_POST" in t and "api/inventory/set" not in t:
            t = t.replace(
                "def do_POST(self):",
                "def do_POST(self):\n        # WP-08 inventory POST routed after body parse if present",
                1,
            )
            # Add at end of file a note - also try send path after json load
            if "json.loads" in t and "api/inventory" not in t.split("def do_POST")[-1][:800]:
                t += '''

# WP-08: if hub uses a generic API router, register these paths in do_POST/do_GET.
# Fallback helper for handlers that call route_api(path, body):
def route_api_inventory(path, body=None):
    if path.startswith("/api/inventory"):
        return _inv_handle(path, body or {})
    return None
'''

    path.write_text(t)
    print("patched", path)
    return True


def main() -> int:
    p = find_server()
    if not p:
        print("sovereign-hub-server not found — copy static panel only")
        return 0
    patch(p)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
