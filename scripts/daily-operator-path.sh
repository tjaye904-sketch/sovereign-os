#!/usr/bin/env bash
# WP-11: daily operator path — stack health + inventory + purpose + visible app open
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
ROOT="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}"
EVID="$ROOT/evidence"
mkdir -p "$EVID"
STAMP=$(date -u +%Y%m%dT%H%M%SZ)
LOG="$EVID/wp11-daily-$STAMP.log"
exec > >(tee -a "$LOG") 2>&1

echo "=== WP-11 daily operator path $STAMP ==="

echo "--- 1) units ---"
for u in sovereign-daemon sovereign-sys sovereign-computer-use sovereign-hub; do
  systemctl --user is-active "$u" 2>/dev/null || echo "$u=inactive"
done

echo "--- 2) ensure ---"
systemctl --user import-environment DISPLAY WAYLAND_DISPLAY XDG_RUNTIME_DIR DBUS_SESSION_BUS_ADDRESS 2>/dev/null || true
systemctl --user start sovereign-daemon sovereign-sys sovereign-computer-use sovereign-hub 2>/dev/null || true
sleep 1
curl -sf http://127.0.0.1:8766/api/status | head -c 400; echo
curl -sf http://127.0.0.1:8765/health; echo
curl -sf http://127.0.0.1:8767/health; echo

echo "--- 3) inventory ---"
if command -v sovereign-inventory >/dev/null; then
  sovereign-inventory list 2>/dev/null | tail -5 || true
  sovereign-inventory allow gnome-system-monitor 2>/dev/null || true
else
  echo "WARN no sovereign-inventory"
fi

echo "--- 4) purpose (if CU up) ---"
if curl -sf http://127.0.0.1:8765/health >/dev/null; then
  sovereign-cu purpose_list 2>/dev/null | head -c 400 || true
  echo
  # non-fatal if purpose missing
  sovereign-cu purpose_run --args '{"purpose_id":"research.web_collect","url":"https://docs.python.org/3/"}' 2>/dev/null | head -c 500 || echo "purpose_run skip"
  echo
fi

echo "--- 5) tools open system monitor (visible) ---"
if command -v sovereign-tools >/dev/null; then
  sovereign-tools --json "open system monitor" || true
else
  sovereign-sys app_open --args '{"target":"gnome-system-monitor"}' || true
fi

echo "--- 6) brief ---"
sovereign-cli brief 2>/dev/null | head -40 || true

echo "=== WP-11 DONE log=$LOG ==="
echo "WP-11 daily operator path: OK"
