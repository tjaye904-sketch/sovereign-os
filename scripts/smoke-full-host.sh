#!/usr/bin/env bash
# WP-20: full host smoke (hub/cu/sys/daemon/shot/tools/purpose)
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
[[ -f "$HOME/.config/sovereign/env" ]] && set -a && . "$HOME/.config/sovereign/env" && set +a
ROOT="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}"
EVID="$ROOT/evidence"
mkdir -p "$EVID"
STAMP=$(date -u +%Y%m%dT%H%M%SZ)
LOG="$EVID/wp20-full-$STAMP.log"
exec > >(tee -a "$LOG") 2>&1

echo "=== WP-20 full host smoke $STAMP ==="
fail=0
pass() { echo "PASS $1"; }
failf() { echo "FAIL $1"; fail=$((fail+1)); }

systemctl --user start sovereign-daemon sovereign-sys sovereign-computer-use sovereign-hub 2>/dev/null || true
sleep 2

systemctl --user is-active sovereign-daemon >/dev/null && pass units-daemon || failf units-daemon
curl -sf http://127.0.0.1:8766/api/status >/dev/null && pass hub || failf hub
curl -sf http://127.0.0.1:8765/health >/dev/null && pass cu || failf cu
curl -sf http://127.0.0.1:8767/health >/dev/null && pass sys || failf sys
curl -sf "${OLLAMA_HOST:-http://127.0.0.1:11434}/api/tags" >/dev/null && pass ollama || failf ollama

if sovereign-cu desktop_screenshot --args '{"name":"wp20.png"}' 2>/dev/null | grep -q '"ok": true'; then
  pass shot
else
  failf shot
fi
if sovereign-tools --json "open system monitor" 2>/dev/null | grep -q '"ok": true'; then
  pass tools
else
  failf tools
fi
if sovereign-cu purpose_list 2>/dev/null | grep -q purpose_id; then
  pass purpose_list
else
  failf purpose_list
fi

# archive pointer
ARCHIVE="$HOME/Downloads/Sovereign-evidence-$STAMP.txt"
{
  echo "WP-20 $STAMP"
  echo "log=$LOG"
  curl -s http://127.0.0.1:8766/api/status || true
} >"$ARCHIVE"
echo "evidence_note=$ARCHIVE"
echo "=== WP-20 done failures=$fail log=$LOG ==="
[[ "$fail" -eq 0 ]]
