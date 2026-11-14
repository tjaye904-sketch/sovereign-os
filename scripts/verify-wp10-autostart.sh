#!/usr/bin/env bash
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
echo "=== WP-10 verify ==="
for u in sovereign-daemon sovereign-sys sovereign-computer-use sovereign-hub; do
  st=$(systemctl --user is-active "$u" 2>/dev/null || echo missing)
  echo "unit $u=$st"
done
curl -sf http://127.0.0.1:8766/api/status | head -c 300; echo
curl -sf http://127.0.0.1:8765/health; echo
curl -sf http://127.0.0.1:8767/health; echo
loginctl show-user "$USER" -p Linger 2>/dev/null || true
echo "WP-10 verify: OK (review unit=active and health JSON)"
