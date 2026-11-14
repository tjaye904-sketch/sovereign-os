#!/usr/bin/env bash
# WP-18: Ollama reachability + plan enrichment status
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
[[ -f "$HOME/.config/sovereign/env" ]] && set -a && . "$HOME/.config/sovereign/env" && set +a

HOST="${OLLAMA_HOST:-http://127.0.0.1:11434}"
MODEL="${SOVEREIGN_OLLAMA_MODEL:-granite4.1:3b}"
TIMEOUT="${SOVEREIGN_OLLAMA_TIMEOUT_S:-120}"

echo "=== WP-18 model enrichment ==="
echo "OLLAMA_HOST=$HOST"
echo "MODEL=$MODEL"
echo "TIMEOUT=${TIMEOUT}s"

if ! curl -sf --connect-timeout 3 "$HOST/api/tags" >/dev/null; then
  echo "FAIL: ollama not reachable at $HOST"
  echo "hint: sudo systemctl start ollama"
  exit 1
fi
echo "PASS: ollama /api/tags"

# short generate probe
CODE=$(curl -s -o /tmp/ollama-probe.json -w "%{http_code}" --max-time 60 \
  "$HOST/api/generate" -d "{\"model\":\"$MODEL\",\"prompt\":\"Reply with exactly PONG\",\"stream\":false}" || true)
echo "generate http=$CODE"
if [[ "$CODE" != "200" ]]; then
  echo "FAIL: generate non-200 — model may need pull: ollama pull $MODEL"
  head -c 300 /tmp/ollama-probe.json; echo
  exit 1
fi
grep -q PONG /tmp/ollama-probe.json && echo "PASS: generate PONG" || echo "WARN: no PONG in body (model still responded)"

# CLI chat
if sovereign-cli chat "Reply with exactly PONG" 2>/dev/null | grep -qi PONG; then
  echo "PASS: sovereign-cli chat PONG"
else
  echo "WARN: cli chat did not show PONG (check login/session not required for chat)"
fi

# Plan enrichment line
DATA=$(mktemp -d)
export SOVEREIGN_DATA_ROOT="$DATA"
sovereign-cli bootstrap owner 'wp18-secret' >/dev/null
sovereign-cli login owner 'wp18-secret' >/dev/null
OUT=$(sovereign-cli plan "WP-18 enrichment probe" 2>&1 || true)
echo "$OUT" | head -20
if echo "$OUT" | grep -q 'enrichment=model_enriched'; then
  echo "PASS: enrichment=model_enriched"
elif echo "$OUT" | grep -q 'enrichment='; then
  echo "WARN: enrichment present but not model_enriched"
  echo "$OUT" | grep enrichment || true
else
  echo "WARN: no enrichment= field (older binary?)"
fi
rm -rf "$DATA"
echo "WP-18 verify done"
