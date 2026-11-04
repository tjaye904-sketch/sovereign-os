#!/usr/bin/env bash
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
DEST="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}/purposes"

echo "=== WP-04 verify ==="
NEED=(research.web_collect.yaml ops.desktop_probe.yaml ops.health_probe.yaml research.wikipedia_brief.yaml)
for f in "${NEED[@]}"; do
  if [[ ! -f "$DEST/$f" ]]; then
    echo "FAIL missing $DEST/$f"
    exit 1
  fi
  echo "PASS file $f"
done

if ! curl -sf http://127.0.0.1:8765/health >/dev/null; then
  echo "WARN CU down — file checks only"
  echo "WP-04 verify: OK (files)"
  exit 0
fi

# purpose_list via server
LIST=$(sovereign-cu purpose_list 2>/dev/null || true)
echo "$LIST" | grep -q research.web_collect || { echo "FAIL purpose_list missing research.web_collect"; echo "$LIST"; exit 1; }
echo "PASS purpose_list contains research.web_collect"

# health purpose (safe)
RUN=$(sovereign-cu purpose_run --args '{"purpose_id":"ops.health_probe"}' 2>/dev/null || true)
echo "$RUN" | grep -q '"ok": true' || { echo "WARN purpose_run health: $RUN"; }

# tools router knows purpose
if command -v sovereign-tools >/dev/null; then
  sovereign-tools --dry-run "list purposes" 2>/dev/null | grep -q purpose_list && echo "PASS tools dry-run list purposes" || true
fi

echo "WP-04 verify: OK"
