#!/usr/bin/env bash
# Sovereign host maturity — remaining ~25–30% automation track.
# Safe on Pop!_OS: no disk wipe, no bare-metal install.
set -euo pipefail

ROOT="${SOVEREIGN_WORK_ROOT:-$HOME/src/sovereign-os/sovereign-work}"
export PATH="$HOME/.local/bin:$PATH"
export OLLAMA_HOST="${OLLAMA_HOST:-http://127.0.0.1:11434}"
export SOVEREIGN_OLLAMA_TIMEOUT_S="${SOVEREIGN_OLLAMA_TIMEOUT_S:-180}"
export SOVEREIGN_DATA_ROOT="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}"

if [[ -f "$HOME/.config/sovereign/env.bash" ]]; then
  # shellcheck disable=SC1091
  source "$HOME/.config/sovereign/env.bash"
elif [[ -f "$HOME/.config/sovereign/env" ]]; then
  set -a
  # shellcheck disable=SC1091
  source "$HOME/.config/sovereign/env"
  set +a
fi

log() { printf '[host-finish] %s\n' "$*"; }
ok()  { printf '[host-finish] OK %s\n' "$*"; }
warn(){ printf '[host-finish] WARN %s\n' "$*" >&2; }

mkdir -p "$HOME/.local/bin" "$HOME/.config/sovereign" \
  "$HOME/.config/systemd/user" "$HOME/.local/share/applications" \
  "$HOME/.local/share/sovereign/purposes" \
  "$HOME/.local/share/sovereign/computer_use/artifacts" \
  "$HOME/.config/autostart"

# --- 0) Env hardening (systemd-safe, no export lines) ---
ENVF="$HOME/.config/sovereign/env"
if [[ ! -f "$ENVF" ]] || grep -q '^export ' "$ENVF" 2>/dev/null; then
  MODEL="${SOVEREIGN_OLLAMA_MODEL:-tripolskypetr/qwen3.5-uncensored-aggressive:4b}"
  EMBED="${SOVEREIGN_EMBED_MODEL:-granite-embedding:latest}"
  cat > "$ENVF" << EOF
OLLAMA_HOST=http://127.0.0.1:11434
SOVEREIGN_OLLAMA_MODEL=$MODEL
SOVEREIGN_EMBED_MODEL=$EMBED
SOVEREIGN_OLLAMA_TIMEOUT_S=180
SOVEREIGN_DATA_ROOT=$HOME/.local/share/sovereign
SOVEREIGN_OWNER_NAME=owner
SOVEREIGN_LISTEN_SECS=5
EOF
  ok "rewrote $ENVF (no export prefixes)"
fi
grep -q SOVEREIGN_OLLAMA_TIMEOUT_S "$ENVF" || echo "SOVEREIGN_OLLAMA_TIMEOUT_S=180" >> "$ENVF"

cat > "$HOME/.config/sovereign/env.bash" << 'EOF'
set -a
# shellcheck disable=SC1091
. "$HOME/.config/sovereign/env"
set +a
export PATH="$HOME/.local/bin:$PATH"
EOF

# --- 1) Ensure binaries ---
CLI="$HOME/.local/bin/sovereign-cli"
DAEMON="$HOME/.local/bin/sovereign-daemon"
if [[ ! -x "$CLI" && -x "$ROOT/build/apps/cli/sovereign-cli" ]]; then
  install -m 755 "$ROOT/build/apps/cli/sovereign-cli" "$CLI"
  ok "installed sovereign-cli from build"
fi
if [[ ! -x "$DAEMON" && -x "$ROOT/build/apps/daemon/sovereign-daemon" ]]; then
  install -m 755 "$ROOT/build/apps/daemon/sovereign-daemon" "$DAEMON"
  ok "installed sovereign-daemon from build"
fi
[[ -x "$CLI" ]] || warn "sovereign-cli missing — build tree first"
[[ -x "$HOME/.local/bin/sovereign-hub-chat" ]] || warn "run WP-21 install for hub chat"

# --- 2) User services: daemon, hub-chat, ensure-ready ---
if [[ -x "$DAEMON" ]]; then
  cat > "$HOME/.config/systemd/user/sovereign-daemon.service" << UNIT
[Unit]
Description=Sovereign OS agent orchestrator (user)
After=default.target

[Service]
Type=simple
EnvironmentFile=-%h/.config/sovereign/env
Environment=PATH=%h/.local/bin:/usr/local/bin:/usr/bin
ExecStart=%h/.local/bin/sovereign-daemon 60
Restart=on-failure
RestartSec=3

[Install]
WantedBy=default.target
UNIT
  systemctl --user daemon-reload
  systemctl --user enable sovereign-daemon.service 2>/dev/null || true
  systemctl --user restart sovereign-daemon.service 2>/dev/null || systemctl --user start sovereign-daemon.service 2>/dev/null || true
  ok "sovereign-daemon user unit"
fi

# ensure-ready oneshot
cat > "$HOME/.local/bin/sovereign-ensure-ready" << 'READY'
#!/usr/bin/env bash
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
systemctl --user start sovereign-daemon 2>/dev/null || true
systemctl --user start sovereign-computer-use 2>/dev/null || true
systemctl --user start sovereign-sys 2>/dev/null || true
systemctl --user start sovereign-hub 2>/dev/null || true
systemctl --user start sovereign-hub-chat 2>/dev/null || true
if ! curl -sf http://127.0.0.1:8765/health >/dev/null 2>&1; then
  if command -v sovereign-cu-server >/dev/null 2>&1; then
    nohup sovereign-cu-server >>"$HOME/.local/share/sovereign/computer_use/server.log" 2>&1 &
  fi
fi
if ! curl -sf http://127.0.0.1:8768/health >/dev/null 2>&1; then
  PY="$HOME/src/sovereign-os/sovereign-work/tools/hub_chat/sovereign_hub_chat_post.py"
  [[ -f "$PY" ]] && nohup python3 "$PY" >>"$HOME/.local/share/sovereign/hub-chat.log" 2>&1 &
fi
exit 0
READY
chmod +x "$HOME/.local/bin/sovereign-ensure-ready"

cat > "$HOME/.config/systemd/user/sovereign-ensure-ready.service" << UNIT
[Unit]
Description=Sovereign ensure daemon/CU/hub/chat ready
After=default.target

[Service]
Type=oneshot
ExecStart=%h/.local/bin/sovereign-ensure-ready
RemainAfterExit=yes

[Install]
WantedBy=default.target
UNIT

cat > "$HOME/.config/autostart/sovereign-ensure-ready.desktop" << EOF
[Desktop Entry]
Type=Application
Name=Sovereign Ensure Ready
Exec=$HOME/.local/bin/sovereign-ensure-ready
X-GNOME-Autostart-enabled=true
NoDisplay=true
EOF

systemctl --user daemon-reload
systemctl --user enable sovereign-ensure-ready.service 2>/dev/null || true
systemctl --user start sovereign-ensure-ready.service 2>/dev/null || "$HOME/.local/bin/sovereign-ensure-ready" || true
loginctl enable-linger "$USER" 2>/dev/null || true
ok "ensure-ready + linger"

# --- 3) Mission Control chat UI (static) ---
HUB_UI="$HOME/.local/share/sovereign/hub-ui"
mkdir -p "$HUB_UI"
cat > "$HUB_UI/index.html" << 'HTML'
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width, initial-scale=1"/>
<title>Sovereign Mission Control</title>
<style>
  :root { --bg:#0f1419; --panel:#1a2332; --text:#e7ecf3; --accent:#3d8bfd; --muted:#8b9bb4; --ok:#3dd68c; --bad:#f07178; }
  * { box-sizing: border-box; }
  body { margin:0; font-family: ui-sans-serif, system-ui, sans-serif; background:var(--bg); color:var(--text); }
  header { padding:1rem 1.25rem; border-bottom:1px solid #243044; display:flex; gap:1rem; align-items:center; flex-wrap:wrap; }
  header h1 { font-size:1.1rem; margin:0; font-weight:600; }
  .pill { font-size:.75rem; padding:.2rem .55rem; border-radius:999px; background:#243044; color:var(--muted); }
  .pill.on { background:#143d2a; color:var(--ok); }
  .pill.off { background:#3d1f24; color:var(--bad); }
  main { display:grid; grid-template-columns: 1fr 320px; gap:1rem; padding:1rem; max-width:1200px; margin:0 auto; }
  @media (max-width:900px){ main { grid-template-columns:1fr; } }
  .card { background:var(--panel); border-radius:12px; padding:1rem; border:1px solid #243044; }
  #log { height:360px; overflow:auto; white-space:pre-wrap; font-family:ui-monospace,monospace; font-size:.85rem; background:#0c1017; padding:.75rem; border-radius:8px; }
  textarea { width:100%; min-height:80px; background:#0c1017; color:var(--text); border:1px solid #243044; border-radius:8px; padding:.6rem; }
  button { background:var(--accent); color:#fff; border:0; border-radius:8px; padding:.55rem 1rem; cursor:pointer; font-weight:600; }
  button.secondary { background:#243044; }
  .row { display:flex; gap:.5rem; flex-wrap:wrap; margin-top:.5rem; }
  #status pre { font-size:.8rem; color:var(--muted); white-space:pre-wrap; }
</style>
</head>
<body>
<header>
  <h1>Sovereign Mission Control</h1>
  <span id="p-daemon" class="pill">daemon</span>
  <span id="p-cu" class="pill">computer-use</span>
  <span id="p-chat" class="pill">chat-api</span>
  <span id="p-ollama" class="pill">ollama</span>
</header>
<main>
  <section class="card">
    <h2 style="margin-top:0;font-size:1rem;">Chat</h2>
    <div id="log"></div>
    <textarea id="prompt" placeholder="Ask Sovereign…"></textarea>
    <div class="row">
      <button id="send">Send</button>
      <button class="secondary" id="pong">PONG test</button>
      <button class="secondary" id="refresh">Refresh status</button>
    </div>
  </section>
  <aside class="card" id="status">
    <h2 style="margin-top:0;font-size:1rem;">Status</h2>
    <pre id="status-body">loading…</pre>
    <div class="row">
      <button class="secondary" id="ensure">Ensure ready</button>
      <button class="secondary" id="open-cu">CU health</button>
    </div>
  </aside>
</main>
<script>
const logEl = document.getElementById('log');
function line(t){ logEl.textContent += t + '\n'; logEl.scrollTop = logEl.scrollHeight; }
async function jget(url){ try { const r = await fetch(url); return await r.json(); } catch(e){ return {ok:false, error:String(e)}; } }
function pill(id, on){ const el=document.getElementById(id); el.classList.toggle('on', !!on); el.classList.toggle('off', !on); }
async function refresh(){
  const st = await jget('http://127.0.0.1:8766/api/status');
  const cu = await jget('http://127.0.0.1:8765/health');
  const chat = await jget('http://127.0.0.1:8768/health');
  const ollama = await jget('http://127.0.0.1:11434/api/tags');
  pill('p-daemon', st.daemon === true || st.ok);
  pill('p-cu', cu.ok === true);
  pill('p-chat', chat.ok === true);
  pill('p-ollama', Array.isArray(ollama.models) || ollama.ok !== false && ollama.models);
  document.getElementById('status-body').textContent = JSON.stringify({hub:st, cu, chat, ollama_models:(ollama.models||[]).map(m=>m.name).slice(0,8)}, null, 2);
}
async function sendPrompt(text){
  line('>>> ' + text);
  try {
    const r = await fetch('http://127.0.0.1:8768/api/chat', {
      method:'POST', headers:{'Content-Type':'application/json'},
      body: JSON.stringify({prompt:text})
    });
    const j = await r.json();
    line(j.text || JSON.stringify(j));
    line('---');
  } catch(e) {
    line('ERROR: ' + e + ' (is sovereign-hub-chat service up on :8768?)');
  }
}
document.getElementById('send').onclick = () => {
  const t = document.getElementById('prompt').value.trim();
  if(t) sendPrompt(t);
};
document.getElementById('pong').onclick = () => sendPrompt('Reply with exactly PONG');
document.getElementById('refresh').onclick = refresh;
document.getElementById('open-cu').onclick = () => jget('http://127.0.0.1:8765/health').then(j=>line(JSON.stringify(j)));
document.getElementById('ensure').onclick = () => line('Run: sovereign-ensure-ready in a terminal');
refresh();
setInterval(refresh, 8000);
</script>
</body>
</html>
HTML

cat > "$HOME/.local/bin/sovereign-mission-control" << EOF
#!/usr/bin/env bash
export PATH="\$HOME/.local/bin:\$PATH"
\$HOME/.local/bin/sovereign-ensure-ready 2>/dev/null || true
UI="\$HOME/.local/share/sovereign/hub-ui/index.html"
if command -v xdg-open >/dev/null; then
  xdg-open "\$UI" 2>/dev/null || xdg-open "file://\$UI"
else
  echo "Open: \$UI"
fi
EOF
chmod +x "$HOME/.local/bin/sovereign-mission-control"

cat > "$HOME/.local/share/applications/sovereign-mission-control.desktop" << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=Sovereign Mission Control
Comment=Chat + status for Sovereign host
Exec=$HOME/.local/bin/sovereign-mission-control
Terminal=false
Categories=Utility;System;
StartupNotify=true
EOF
ok "Mission Control UI → chat API :8768"

# --- 4) Purpose pack sync (C++ + CU shared YAML dir) ---
for f in ops.desktop_probe.yaml research.web_collect.yaml; do
  if [[ -f "$ROOT/purposes/$f" ]]; then
    cp -a "$ROOT/purposes/$f" "$HOME/.local/share/sovereign/purposes/" 2>/dev/null || true
  fi
done
# minimal packs if missing
if [[ ! -f "$HOME/.local/share/sovereign/purposes/research.web_collect.yaml" ]]; then
  cat > "$HOME/.local/share/sovereign/purposes/research.web_collect.yaml" << 'YAML'
purpose_id: research.web_collect
description: Open allowlisted docs URL, snapshot, screenshot (headed)
risk: low
require_headed: true
allowed_hosts:
  - docs.python.org
  - docs.github.com
  - en.wikipedia.org
  - manpages.ubuntu.com
  - github.com
steps:
  - action: browser_open
    target: "{url}"
  - action: browser_snapshot
  - action: browser_screenshot
YAML
fi
ok "purpose packs in data_root/purposes"

# --- 5) Ollama + plan enrichment probe ---
if curl -sf "$OLLAMA_HOST/api/tags" >/dev/null 2>&1; then
  ok "ollama reachable"
else
  warn "ollama down — start: sudo systemctl start ollama"
fi

if [[ -x "$CLI" ]]; then
  # Prefer existing session data root; don't destroy user data
  set +e
  "$CLI" status 2>/dev/null | head -5
  out="$("$CLI" plan "Host finish enrichment probe $(date +%s)" 2>&1)"
  set -e
  echo "$out" | tail -20
  if echo "$out" | grep -q 'enrichment=model_enriched'; then
    ok "plan enrichment live"
  elif echo "$out" | grep -q 'enrichment='; then
    warn "plan enrichment=$(echo "$out" | sed -n 's/^enrichment=//p' | head -1) — chat may still work; see docs"
  fi
  set +e
  chat="$("$HOME/.local/bin/sovereign-hub-chat" "Reply with exactly PONG" 2>&1)"
  set -e
  echo "$chat" | tail -5
fi

# --- 6) CTest if build exists ---
if [[ -d "$ROOT/build" ]]; then
  log "running ctest (quick)"
  ctest --test-dir "$ROOT/build" --output-on-failure -j"$(nproc)" || warn "ctest reported failures"
fi

# --- 7) Evidence ---
EV="$HOME/.local/share/sovereign/evidence/host-finish-$(date -u +%Y%m%dT%H%M%SZ).txt"
mkdir -p "$(dirname "$EV")"
{
  echo "time=$(date -Is)"
  echo "root=$ROOT"
  systemctl --user is-active sovereign-daemon 2>/dev/null || true
  curl -s http://127.0.0.1:8765/health 2>/dev/null || true
  curl -s http://127.0.0.1:8768/health 2>/dev/null || true
  curl -s http://127.0.0.1:8766/api/status 2>/dev/null || true
  "$CLI" status 2>/dev/null || true
} > "$EV" 2>&1 || true
ok "evidence $EV"

log "DONE — open Mission Control: sovereign-mission-control"
log "Chat API: curl -s http://127.0.0.1:8768/health"
exit 0
