#!/usr/bin/env bash
set -euo pipefail

BIN_DIR="${HOME}/.local/bin"
HUB_DIR="${HOME}/.local/share/sovereign-hub"
CLI="${SOVEREIGN_CLI:-$(pwd)/build/apps/cli/sovereign-cli}"
[ -x "$CLI" ] || CLI="${BIN_DIR}/sovereign-cli"

export SOVEREIGN_DATA_ROOT="${SOVEREIGN_DATA_ROOT:-${HOME}/.local/share/sovereign}"

# 1. sovereign-daemon
if ! pgrep -f "sovereign-daemon" >/dev/null 2>&1; then
  systemctl --user start sovereign-daemon >/dev/null 2>&1 || nohup "${BIN_DIR}/sovereign-daemon" 60 >/dev/null 2>&1 &
fi

# 2. computer_use (8765)
if ! curl -fsS --max-time 1 "http://127.0.0" >/dev/null 2>&1 && [ -x "${BIN_DIR}/sovereign-cu-server" ]; then
  nohup "${BIN_DIR}/sovereign-cu-server" >/dev/null 2>&1 &
fi

# 3. sys (8767)
if ! curl -fsS --max-time 1 "http://127.0.0" >/dev/null 2>&1 && [ -x "${BIN_DIR}/sovereign-sys-server" ]; then
  nohup "${BIN_DIR}/sovereign-sys-server" >/dev/null 2>&1 &
fi

# 4. hub (8766)
if ! curl -fsS --max-time 1 "http://127.0.0" >/dev/null 2>&1 && [ -x "${BIN_DIR}/sovereign-hub-server" ]; then
  nohup "${BIN_DIR}/sovereign-hub-server" >/dev/null 2>&1 &
fi

# 5. caps sidecar (8769)
if ! curl -fsS --max-time 1 "http://127.0.0" >/dev/null 2>&1 && [ -f "${HUB_DIR}/caps_sidecar.py" ]; then
  nohup python3 "${HUB_DIR}/caps_sidecar.py" >/dev/null 2>&1 &
fi

sleep 1
exec "$CLI" session-ensure --json
