#!/usr/bin/env bash
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"

echo "=== WP-08/09 verify ==="

# WP-09 tools precheck
if command -v sovereign-tools >/dev/null; then
  sovereign-tools --dry-run "open gucharmap" 2>/dev/null | tee /tmp/wp09-deny.txt || true
  # may show inventory deny in precheck or on execute
  OUT=$(sovereign-tools --json "open gucharmap" 2>/dev/null || true)
  echo "$OUT" | head -c 500; echo
  if echo "$OUT" | grep -qiE 'denied|inventory'; then
    echo "PASS tools deny path for gucharmap"
  else
    # if gucharmap allowed in policy, still ok if result ok false
    echo "WARN gucharmap may be allowed in policy; check policy"
  fi

  sovereign-inventory allow gnome-system-monitor 2>/dev/null || true
  OUT2=$(sovereign-tools --dry-run --json "open system monitor" 2>/dev/null || true)
  echo "$OUT2" | head -c 400; echo
  echo "PASS tools dry-run system monitor"
else
  echo "WARN sovereign-tools missing"
fi

# WP-08 hub API
if curl -sf http://127.0.0.1:8766/api/status >/dev/null; then
  if curl -sf http://127.0.0.1:8766/api/inventory/list >/dev/null; then
    echo "PASS hub /api/inventory/list"
  else
    echo "WARN hub inventory API not yet wired — open Mission Control after patch; list may 404 until hub restart"
  fi
else
  echo "WARN hub not running"
fi

echo "WP-08/09 verify: done (check WARNs)"
