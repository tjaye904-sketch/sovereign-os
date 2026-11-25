#!/usr/bin/env bash
set -euo pipefail

BIN_DIR="${HOME}/.local/bin"
HUB_DIR="${HOME}/.local/share/sovereign-hub"
CLI="${SOVEREIGN_CLI:-$(pwd)/build/apps/cli/sovereign-cli}"
[ -x "$CLI" ] || CLI="${BIN_DIR}/sovereign-cli"
export SOVEREIGN_DATA_ROOT="${SOVEREIGN_DATA_ROOT:-${HOME}/.local/share/sovereign}"

if ! pgrep -f "sovereign-daemon" >/dev/null 2>&1; then
  systemctl --user start sovereign-daemon >/dev/null 2>&1 || nohup "${BIN_DIR}/sovereign-daemon" 60 >/dev/null 2>&1 &
fi
if ! curl -4 -fsS --noproxy "*" --max-time 1 "http://localhost:8765/health" >/dev/null 2>&1 && [ -x "${BIN_DIR}/sovereign-cu-server" ]; then
  nohup "${BIN_DIR}/sovereign-cu-server" >/dev/null 2>&1 &
fi
if ! curl -4 -fsS --noproxy "*" --max-time 1 "http://localhost:8767/health" >/dev/null 2>&1 && [ -x "${BIN_DIR}/sovereign-sys-server" ]; then
  nohup "${BIN_DIR}/sovereign-sys-server" >/dev/null 2>&1 &
fi
if ! curl -4 -fsS --noproxy "*" --max-time 1 "http://localhost:8766/api/status" >/dev/null 2>&1 && [ -x "${BIN_DIR}/sovereign-hub-server" ]; then
  nohup "${BIN_DIR}/sovereign-hub-server" >/dev/null 2>&1 &
fi
if ! curl -4 -fsS --noproxy "*" --max-time 1 "http://localhost:8769/health" >/dev/null 2>&1 && [ -f "${HUB_DIR}/caps_sidecar.py" ]; then
  nohup python3 "${HUB_DIR}/caps_sidecar.py" >/dev/null 2>&1 &
fi

sleep 1
exec "$CLI" session-ensure --json
