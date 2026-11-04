#!/usr/bin/env bash
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
[[ -f "${HOME}/.config/sovereign/env" ]] && set -a && . "${HOME}/.config/sovereign/env" && set +a
export SOVEREIGN_CU_HEADLESS="${SOVEREIGN_CU_HEADLESS:-0}"
export SOVEREIGN_SYS_HEADLESS="${SOVEREIGN_SYS_HEADLESS:-0}"
export SOVEREIGN_DATA_ROOT="${SOVEREIGN_DATA_ROOT:-${HOME}/.local/share/sovereign}"
mkdir -p "${SOVEREIGN_DATA_ROOT}/computer_use" "${SOVEREIGN_DATA_ROOT}/logs"

log() { echo "[sovereign-session] $*" | tee -a "${SOVEREIGN_DATA_ROOT}/logs/session-start.log"; }
log "start $(date -Is)"

systemctl --user import-environment \
  DISPLAY WAYLAND_DISPLAY XDG_RUNTIME_DIR DBUS_SESSION_BUS_ADDRESS XAUTHORITY 2>/dev/null || true
systemctl --user daemon-reload 2>/dev/null || true

systemctl --user start sovereign-daemon.service 2>/dev/null || true
systemctl --user start sovereign-hub.service 2>/dev/null || true
systemctl --user start sovereign-sys.service 2>/dev/null || true

# CU
systemctl --user stop sovereign-computer-use.service 2>/dev/null || true
pkill -f '[s]overeign-cu-server' 2>/dev/null || true
sleep 0.3
systemctl --user start sovereign-computer-use.service 2>/dev/null || true
sleep 1
if ! curl -sf --connect-timeout 2 http://127.0.0.1:8765/health >/dev/null 2>&1; then
  log "CU fallback nohup"
  nohup env SOVEREIGN_CU_HEADLESS=0 "${HOME}/.local/bin/sovereign-cu-server" \
    >"${SOVEREIGN_DATA_ROOT}/computer_use/server.log" 2>&1 &
  sleep 1
fi

# SYS — unit often fails if package missing; fall back
sleep 0.5
if ! curl -sf --connect-timeout 2 http://127.0.0.1:8767/health >/dev/null 2>&1; then
  log "SYS fallback nohup"
  systemctl --user stop sovereign-sys.service 2>/dev/null || true
  pkill -f '[s]overeign-sys-server' 2>/dev/null || true
  nohup env SOVEREIGN_SYS_HEADLESS=0 "${HOME}/.local/bin/sovereign-sys-server" \
    >"${SOVEREIGN_DATA_ROOT}/logs/sys-server.log" 2>&1 &
  sleep 1
fi

# HUB
if ! curl -sf --connect-timeout 2 http://127.0.0.1:8766/api/status >/dev/null 2>&1; then
  log "HUB fallback nohup"
  nohup "${HOME}/.local/bin/sovereign-hub-server" \
    >"${SOVEREIGN_DATA_ROOT}/hub.log" 2>&1 &
  sleep 1
fi

for u in 8765/health 8766/api/status 8767/health; do
  if curl -sf --connect-timeout 2 "http://127.0.0.1:$u" >/dev/null; then log "OK $u"; else log "DOWN $u"; fi
done
log "done $(date -Is)"
