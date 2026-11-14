#!/usr/bin/env bash
# WP-16: exercise hub-adjacent APIs (purpose + inventory) for Mission Control wiring
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
echo "=== WP-16 hub API actions ==="
curl -sf http://127.0.0.1:8766/api/status | head -c 250; echo
echo "--- purpose_list via CU ---"
sovereign-cu purpose_list | head -c 500; echo
echo "--- inventory summary ---"
sovereign-inventory list 2>/dev/null | awk '/allowed|ALLOW|DENY/{c++} END{print "lines",c+0}'
sovereign-inventory list 2>/dev/null | grep ALLOW | head -8 || true
echo "--- hub open ---"
if command -v sovereign-hub-open >/dev/null; then
  # non-blocking
  nohup sovereign-hub-open >/dev/null 2>&1 &
  sleep 1
  echo "hub-open launched"
fi
echo "WP-16 hub API actions: OK"
