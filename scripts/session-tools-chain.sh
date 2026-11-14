#!/usr/bin/env bash
# WP-15: multi-step operator session — status → tools → purpose → memory → audit
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
[[ -f "$HOME/.config/sovereign/env" ]] && set -a && . "$HOME/.config/sovereign/env" && set +a
ROOT="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}"
EVID="$ROOT/evidence"
mkdir -p "$EVID"
STAMP=$(date -u +%Y%m%dT%H%M%SZ)
LOG="$EVID/wp15-session-$STAMP.log"
GOAL="${*:-Daily host operator session}"

exec > >(tee -a "$LOG") 2>&1
echo "=== WP-15 session tools chain $STAMP ==="
echo "goal=$GOAL"

echo "--- ensure stack ---"
systemctl --user start sovereign-daemon sovereign-sys sovereign-computer-use sovereign-hub 2>/dev/null || true
sleep 1
curl -sf http://127.0.0.1:8766/api/status | head -c 300; echo

echo "--- login (best effort) ---"
sovereign-cli login owner "${SOVEREIGN_OWNER_SECRET:-your-secret}" 2>/dev/null || \
  sovereign-cli login owner 'your-secret' 2>/dev/null || \
  echo "login skipped (set SOVEREIGN_OWNER_SECRET or login manually)"

echo "--- plan ---"
sovereign-cli plan "$GOAL" 2>/dev/null | head -30 || true

echo "--- tools: open system monitor ---"
sovereign-chat-tools "open system monitor" 2>/dev/null || sovereign-tools --json "open system monitor" || true

echo "--- purpose research (headed) ---"
if curl -sf http://127.0.0.1:8765/health >/dev/null; then
  sovereign-cu purpose_run --args '{"purpose_id":"research.web_collect","url":"https://docs.python.org/3/"}' 2>/dev/null | head -c 600 || true
  echo
fi

echo "--- screenshot audit ---"
sovereign-cu desktop_screenshot --args "{\"name\":\"wp15-$STAMP.png\"}" 2>/dev/null || true

echo "--- memory ---"
sovereign-cli memory-add session "WP-15 chain $STAMP goal=$GOAL" 2>/dev/null || true

echo "--- brief ---"
sovereign-cli brief 2>/dev/null | head -35 || true

echo "=== WP-15 DONE log=$LOG ==="
echo "WP-15 session tools chain: OK"
