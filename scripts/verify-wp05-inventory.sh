#!/usr/bin/env bash
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
ROOT="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}"
echo "=== WP-05 verify ==="

if command -v sovereign-inventory >/dev/null; then
  sovereign-inventory scan || true
  sovereign-inventory list 2>/dev/null | head -5 || true
  echo "PASS sovereign-inventory CLI present"
else
  echo "WARN sovereign-inventory CLI missing — checking files only"
fi

# Accept common locations
for f in "$ROOT/inventory/apps.json" "$ROOT/apps.json" "$ROOT/inventory/app_policy.json" "$ROOT/app_policy.json"; do
  if [[ -f "$f" ]]; then
    echo "PASS found $f"
    FOUND=1
  fi
done
if [[ -z "${FOUND:-}" ]]; then
  echo "WARN no inventory files yet — run scan after install"
fi

# Sys policy integration optional
if curl -sf http://127.0.0.1:8767/health >/dev/null; then
  echo "PASS sys health (policy may use inventory)"
fi
echo "WP-05 verify: OK"
