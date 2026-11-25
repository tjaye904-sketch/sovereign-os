#!/usr/bin/env python3
from __future__ import annotations
import json, sys
from http.server import BaseHTTPRequestHandler, HTTPServer
from pathlib import Path
from urllib.parse import urlparse, parse_qs
sys.path.insert(0, str(Path(__file__).parent))
from capability_api import list_capabilities, set_capability, gate_check, desktop_status, federation_status, list_pending_decisions, decide_governance, fleet_status, observe_status, curiosity_status, inventory_status, purposes_status

class H(BaseHTTPRequestHandler):
    def log_message(self, *a): pass
    def _json(self, code, obj):
        b = json.dumps(obj).encode()
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Content-Length", str(len(b)))
        self.end_headers()
        self.wfile.write(b)
    def do_OPTIONS(self):
        self.send_response(204)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET,POST,OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()
    def do_GET(self):
        path = urlparse(self.path).path.rstrip("/") or "/"
        if path in ("/", "/health"):
            return self._json(200, {"ok": True, "service": "sovereign-caps", "port": 8769})
        if path == "/api/capabilities":
            return self._json(200, list_capabilities())
        if path == "/api/desktop-status":
            return self._json(200, desktop_status())
        if path == "/api/federation":
            return self._json(200, federation_status())
        if path == "/api/fleet":
            return self._json(200, fleet_status())
        if path == "/api/observe":
            return self._json(200, observe_status())
        if path == "/api/curiosity":
            return self._json(200, curiosity_status())
        if path == "/api/inventory":
            return self._json(200, inventory_status())
        if path == "/api/purposes":
            return self._json(200, purposes_status())
        if path == "/api/governance/pending":
            return self._json(200, list_pending_decisions())
        if path == "/api/gate-check":
            q = parse_qs(urlparse(self.path).query)
            act = (q.get("action") or ["browser_open"])[0]
            return self._json(200, gate_check(act))
        if path == "/ui":
            html = open(Path(__file__).parent / "ui.html").read() if (Path(__file__).parent / "ui.html").exists() else """<!DOCTYPE html><html><body style="background:#0f1419;color:#eee;font-family:system-ui">
<h1>Sovereign Capabilities</h1><pre id=o>loading</pre>
<script>
async function load(){const j=await(await fetch('/api/capabilities')).json();
let h='ok='+j.ok+'\\n';
for(const c of j.capabilities||[]){h+=c.id+' enabled='+c.enabled+'\\n';}
document.getElementById('o').textContent=h||JSON.stringify(j,null,2);}
load();
</script></body></html>"""
            b = html.encode()
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(b)))
            self.end_headers()
            self.wfile.write(b)
            return
        return self._json(404, {"ok": False, "error": "not found", "path": path})
    def do_POST(self):
        path = urlparse(self.path).path.rstrip("/")
        n = int(self.headers.get("Content-Length") or 0)
        try:
            body = json.loads(self.rfile.read(n).decode() or "{}")
        except Exception:
            body = {}
        if path.startswith("/api/governance/"):
            did = path.split("/api/governance/", 1)[1]
            approve = str(body.get("decision", "approve")).lower() == "approve"
            return self._json(200, decide_governance(did, approve, str(body.get("reason", "mission-control"))))
        if path.startswith("/api/capabilities/"):
            cap_id = path.split("/api/capabilities/", 1)[1]
            return self._json(200, set_capability(cap_id, bool(body.get("enabled", True))))
        return self._json(404, {"ok": False, "error": "not found", "path": path})

if __name__ == "__main__":
    print("sovereign-caps on 8769", flush=True)
    HTTPServer(("127.0.0.1", 8769), H).serve_forever()
