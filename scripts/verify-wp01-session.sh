#!/usr/bin/env bash
# WP-01 smoke (run after session-start; full reboot test is manual twice)
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
FAIL=0

check_url() {
  local u="$1"
  if curl -sf --connect-timeout 3 "$u" >/dev/null; then
    echo "PASS $u"
  else
    echo "FAIL $u"
    FAIL=1
  fi
}

echo "=== WP-01 verify ==="
check_url http://127.0.0.1:8765/health
check_url http://127.0.0.1:8766/api/status
check_url http://127.0.0.1:8767/health

if systemctl --user is-active --quiet sovereign-daemon.service 2>/dev/null; then
  echo "PASS sovereign-daemon active"
else
  echo "FAIL sovereign-daemon not active"
  FAIL=1
fi

# Optional: headed CU open (skip if offline)
if curl -sf http://127.0.0.1:8765/health >/dev/null; then
  if command -v sovereign-cu >/dev/null; then
    if sovereign-cu browser.open --args '{"url":"https://docs.python.org/3/"}' 2>/dev/null | grep -q '"ok": true'; then
      echo "PASS browser.open"
    else
      echo "WARN browser.open failed (display/CU policy?)"
    fi
  fi
fi

if [[ "$FAIL" -eq 0 ]]; then
  echo "WP-01 verify: OK"
  exit 0
fi
echo "WP-01 verify: FAILED"
exit 1
