#!/usr/bin/env bash
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
systemctl --user status sovereign-daemon --no-pager -l 2>/dev/null | head -25 || true
if [[ -x "$HOME/src/sovereign-os/sovereign-work/scripts/sovereign-ctl.sh" ]]; then
  bash "$HOME/src/sovereign-os/sovereign-work/scripts/sovereign-ctl.sh" status 2>/dev/null || true
elif command -v sovereign-ctl >/dev/null; then
  sovereign-ctl status 2>/dev/null || true
else
  echo "SOFT no sovereign-ctl — systemctl path used"
fi
echo "smoke-ctl: OK"
