#!/usr/bin/env bash
# Gap 9: one entrypoint — CU server if up, else sovereign-cli purpose-run
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
PURPOSE_ID="${1:-research.web_collect}"
URL="${2:-https://docs.python.org/3/}"
ARGS=$(printf '{"purpose_id":"%s","url":"%s"}' "$PURPOSE_ID" "$URL")

if curl -sf http://127.0.0.1:8765/health >/dev/null 2>&1; then
  echo "[unified] via computer-use server"
  if command -v sovereign-cu >/dev/null; then
    sovereign-cu purpose_run --args "$ARGS" || \
      curl -s -X POST http://127.0.0.1:8765/v1/action \
        -H 'Content-Type: application/json' \
        -d "{\"action\":\"purpose_run\",\"args\":$ARGS}"
    exit 0
  fi
fi

CLI=$(command -v sovereign-cli || true)
[[ -x "$CLI" ]] || CLI="$HOME/src/sovereign-os/sovereign-work/build/apps/cli/sovereign-cli"
if [[ -x "$CLI" ]] && "$CLI" 2>&1 | grep -q purpose-run; then
  echo "[unified] via sovereign-cli purpose-run"
  exec "$CLI" purpose-run "$PURPOSE_ID" --url "$URL"
fi
echo "FAIL: neither CU server nor CLI purpose-run available" >&2
exit 1
