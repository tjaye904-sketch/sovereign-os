#!/usr/bin/env python3
"""Patch Mission Control hub server to expose /api/capabilities (+ optional UI snippet)."""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(sys.argv[1] if len(sys.argv) > 1 else Path.home() / "src/sovereign-os/sovereign-work")
CLIENT = ROOT / "integrations/hub/capability_api.py"
if not CLIENT.exists():
    CLIENT = Path(__file__).resolve().parents[1] / "hub/capability_api.py"

# Find hub server
cands = [
    Path.home() / ".local/bin/sovereign-hub-server",
    Path.home() / ".local/share/sovereign-hub/server.py",
    ROOT / "integrations/hub/server.py",
]
# also search
for p in Path.home().joinpath(".local").rglob("*hub*server*"):
    if p.is_file() and p.suffix in (".py", "") and p.stat().st_size < 500_000:
        cands.append(p)

server = None
for c in cands:
    if not c.exists():
        continue
    try:
        t = c.read_text(errors="ignore") if c.suffix == ".py" or "python" in open(c).readline() else ""
    except Exception:
        continue
    if c.suffix == ".py" and ("8766" in t or "api/status" in t or "Mission" in t):
        server = c
        break
    # wrapper script
    if c.name == "sovereign-hub-server":
        text = c.read_text(errors="ignore")
        m = re.search(r"([/\w.-]+server\.py)", text)
        if m and Path(m.group(1)).exists():
            server = Path(m.group(1))
            break
        # exec python -c or module
        print("wrapper:", c)
        print(text[:400])

# Install API module next to hub if we find a package dir
hub_pkg = Path.home() / ".local/share/sovereign-hub"
hub_pkg.mkdir(parents=True, exist_ok=True)
api_dst = hub_pkg / "capability_api.py"
api_dst.write_text(
    (ROOT / "integrations/hub/capability_api.py").read_text()
    if (ROOT / "integrations/hub/capability_api.py").exists()
    else Path(sys.argv[0]).parent.parent.joinpath("hub/capability_api.py").read_text()
)
print("wrote", api_dst)

# Also install a tiny sidecar HTTP app if main hub can't be patched
sidecar = hub_pkg / "caps_sidecar.py"
sidecar.write_text(r'''#!/usr/bin/env python3
"""L5 sidecar on 8768 — capability API if main hub is opaque."""
from __future__ import annotations
import json
from http.server import BaseHTTPRequestHandler, HTTPServer
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))
from capability_api import list_capabilities, set_capability, gate_check

class H(BaseHTTPRequestHandler):
    def log_message(self, *a):
        pass
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
        if self.path.startswith("/api/capabilities"):
            return self._json(200, list_capabilities())
        if self.path.startswith("/api/gate-check"):
            from urllib.parse import urlparse, parse_qs
            q = parse_qs(urlparse(self.path).query)
            act = (q.get("action") or ["browser_open"])[0]
            return self._json(200, gate_check(act))
        if self.path in ("/", "/health"):
            return self._json(200, {"ok": True, "service": "sovereign-caps", "port": 8768})
        # minimal UI
        if self.path.startswith("/ui"):
            html = """<!DOCTYPE html><html><head><meta charset=utf-8>
<title>Sovereign Capabilities</title>
<style>
body{font-family:system-ui;background:#0f1419;color:#e7ecf3;margin:24px}
table{border-collapse:collapse;width:100%;max-width:960px}
th,td{border:1px solid #2a3544;padding:8px 10px;text-align:left}
th{background:#1a2332}
button{cursor:pointer;padding:4px 10px;border-radius:6px;border:0}
.on{background:#3d9a5f;color:#fff}.off{background:#8b3a3a;color:#fff}
h1{font-size:1.25rem} .muted{color:#8b9bb4;font-size:.9rem}
</style></head><body>
<h1>Sovereign — Capabilities</h1>
<p class=muted>L5 Mission Control panel · data from sovereign-cli</p>
<div id=status></div>
<table><thead><tr><th>id</th><th>enabled</th><th>source</th><th>description</th><th></th></tr></thead>
<tbody id=tb></tbody></table>
<script>
async function load(){
  const r=await fetch('/api/capabilities'); const j=await r.json();
  document.getElementById('status').textContent=j.ok?'ok':'error '+(j.error||'');
  const tb=document.getElementById('tb'); tb.innerHTML='';
  for(const c of (j.capabilities||[])){
    const tr=document.createElement('tr');
    tr.innerHTML=`<td><code>${c.id}</code></td><td>${c.enabled}</td><td>${c.source||''}</td>
      <td>${c.description||''}</td>
      <td><button class="${c.enabled?'on':'off'}" data-id="${c.id}" data-en="${c.enabled?0:1}">
      ${c.enabled?'Disable':'Enable'}</button></td>`;
    tb.appendChild(tr);
  }
  tb.querySelectorAll('button').forEach(b=>b.onclick=async()=>{
    await fetch('/api/capabilities/'+encodeURIComponent(b.dataset.id),{
      method:'POST', headers:{'Content-Type':'application/json'},
      body:JSON.stringify({enabled: b.dataset.en==='1'})
    });
    load();
  });
}
load(); setInterval(load, 8000);
</script></body></html>"""
            b = html.encode()
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(b)))
            self.end_headers()
            self.wfile.write(b)
            return
        self._json(404, {"ok": False, "error": "not found"})
    def do_POST(self):
        n = int(self.headers.get("Content-Length") or 0)
        raw = self.rfile.read(n) if n else b"{}"
        try:
            body = json.loads(raw.decode() or "{}")
        except Exception:
            body = {}
        path = self.path.rstrip("/")
        if path.startswith("/api/capabilities/"):
            cap_id = path.split("/api/capabilities/", 1)[1]
            en = bool(body.get("enabled", True))
            return self._json(200, set_capability(cap_id, en))
        self._json(404, {"ok": False})

if __name__ == "__main__":
    HTTPServer(("127.0.0.1", 8768), H).serve_forever()
''')
sidecar.chmod(0o755)
print("sidecar", sidecar)

# launcher
launch = Path.home() / ".local/bin/sovereign-caps-ui"
launch.write_text(f'''#!/usr/bin/env bash
export SOVEREIGN_DATA_ROOT="${{SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}}"
export SOVEREIGN_CLI="${{SOVEREIGN_CLI:-$HOME/src/sovereign-os/sovereign-work/build/apps/cli/sovereign-cli}}"
export PATH="$HOME/.local/bin:$PATH"
pkill -f caps_sidecar.py 2>/dev/null || true
sleep 0.3
nohup python3 "{sidecar}" >"$SOVEREIGN_DATA_ROOT/caps_ui.log" 2>&1 &
sleep 0.5
url=http://127.0.0.1:8768/ui
if command -v xdg-open >/dev/null; then xdg-open "$url" 2>/dev/null & fi
echo "Capabilities UI: $url"
echo "API: http://127.0.0.1:8768/api/capabilities"
''')
launch.chmod(0o755)
print("launcher", launch)

# systemd user unit
unit = Path.home() / ".config/systemd/user/sovereign-caps-ui.service"
unit.parent.mkdir(parents=True, exist_ok=True)
unit.write_text(f'''[Unit]
Description=Sovereign L5 capability panel (8768)
After=default.target

[Service]
Type=simple
Environment=SOVEREIGN_DATA_ROOT=%h/.local/share/sovereign
Environment=SOVEREIGN_CLI=%h/src/sovereign-os/sovereign-work/build/apps/cli/sovereign-cli
ExecStart=/usr/bin/python3 {sidecar}
Restart=on-failure

[Install]
WantedBy=default.target
''')
print("unit", unit)
print("patch-hub-capabilities: OK")
