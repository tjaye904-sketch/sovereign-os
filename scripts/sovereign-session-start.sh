#!/usr/bin/env bash
# WP-01: Bring Sovereign operators up with graphical session env.
# Run from desktop login (autostart) or manually from a GUI terminal.
set -euo pipefail

export PATH="${HOME}/.local/bin:${PATH}"

if [[ -f "${HOME}/.config/sovereign/env" ]]; then
  set -a
  # shellcheck disable=SC1091
  . "${HOME}/.config/sovereign/env"
  set +a
fi

export SOVEREIGN_CU_HEADLESS="${SOVEREIGN_CU_HEADLESS:-0}"
export SOVEREIGN_SYS_HEADLESS="${SOVEREIGN_SYS_HEADLESS:-0}"
export SOVEREIGN_DATA_ROOT="${SOVEREIGN_DATA_ROOT:-${HOME}/.local/share/sovereign}"
mkdir -p "${SOVEREIGN_DATA_ROOT}/computer_use" "${SOVEREIGN_DATA_ROOT}/logs"

log() { echo "[sovereign-session] $*" | tee -a "${SOVEREIGN_DATA_ROOT}/logs/session-start.log"; }

log "start $(date -Is)"

# Critical: user systemd must see the graphical session
systemctl --user import-environment \
  DISPLAY WAYLAND_DISPLAY XDG_RUNTIME_DIR DBUS_SESSION_BUS_ADDRESS XAUTHORITY \
  2>/dev/null || true

systemctl --user daemon-reload 2>/dev/null || true

start_unit() {
  local u="$1"
  systemctl --user start "${u}" 2>/dev/null || log "warn: could not start ${u}"
}

start_unit sovereign-daemon.service
start_unit sovereign-sys.service
start_unit sovereign-hub.service

# Computer-use: prefer unit after env import; fall back to session-scoped process
systemctl --user stop sovereign-computer-use.service 2>/dev/null || true
pkill -f '[s]overeign-cu-server' 2>/dev/null || true
sleep 0.4

start_unit sovereign-computer-use.service
sleep 1

if ! curl -sf --connect-timeout 2 http://127.0.0.1:8765/health >/dev/null 2>&1; then
  log "CU unit not healthy — starting session-scoped sovereign-cu-server"
  pkill -f '[s]overeign-cu-server' 2>/dev/null || true
  nohup env SOVEREIGN_CU_HEADLESS=0 PATH="${PATH}" \
    "${HOME}/.local/bin/sovereign-cu-server" \
    >"${SOVEREIGN_DATA_ROOT}/computer_use/server.log" 2>&1 &
  sleep 1
fi

if ! curl -sf --connect-timeout 2 http://127.0.0.1:8766/api/status >/dev/null 2>&1; then
  if [[ -x "${HOME}/.local/bin/sovereign-hub-server" ]]; then
    log "hub not up — session-scoped hub"
    nohup "${HOME}/.local/bin/sovereign-hub-server" \
      >"${SOVEREIGN_DATA_ROOT}/hub.log" 2>&1 &
    sleep 1
  fi
fi

# Summary
for url in \
  http://127.0.0.1:8765/health \
  http://127.0.0.1:8766/api/status \
  http://127.0.0.1:8767/health
do
  if curl -sf --connect-timeout 2 "$url" >/dev/null 2>&1; then
    log "OK $url"
  else
    log "DOWN $url"
  fi
done

log "done $(date -Is)"
