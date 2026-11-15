#!/usr/bin/env bash
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
if [[ -f "${HOME}/.config/sovereign/env.bash" ]]; then
  # shellcheck disable=SC1091
  source "${HOME}/.config/sovereign/env.bash"
elif [[ -f "${HOME}/.config/sovereign/env" ]]; then
  set -a
  # shellcheck disable=SC1091
  source "${HOME}/.config/sovereign/env"
  set +a
fi
CLI="${HOME}/.local/bin/sovereign-cli"
[[ -x "$CLI" ]] || CLI="${HOME}/src/sovereign-os/sovereign-work/build/apps/cli/sovereign-cli"
[[ -x "$CLI" ]] || { echo "FAIL: sovereign-cli missing"; exit 1; }

DATA="${SOVEREIGN_DATA_ROOT:-}"
CLEAN=0
if [[ -z "$DATA" ]]; then
  DATA="$(mktemp -d)"
  export SOVEREIGN_DATA_ROOT="$DATA"
  CLEAN=1
fi

"$CLI" bootstrap owner 'wp21-secret' 2>/dev/null || true
"$CLI" login owner 'wp21-secret' 2>/dev/null || "$CLI" login owner 'your-secret' 2>/dev/null || true

out="$("$CLI" plan "WP21 verify enrichment honesty" 2>&1 || true)"
echo "$out"
if ! echo "$out" | grep -q 'enrichment='; then
  echo "FAIL: no enrichment= field" >&2
  exit 1
fi
status="$(echo "$out" | sed -n 's/^enrichment=//p' | head -1)"
case "$status" in
  model_enriched|model_unavailable|heuristic|no_router|parse_failed|timeout|model_failed)
    echo "verify-wp21-plan-enrichment: OK status=$status"
    ;;
  *)
    echo "FAIL: unexpected enrichment status: $status" >&2
    exit 1
    ;;
esac
# Dishonest path: model name alone must not force enriched without chat
if echo "$out" | grep -q 'enrichment=model_enriched' && echo "$out" | grep -qi 'model chat failed'; then
  echo "FAIL: model_enriched with chat failed" >&2
  exit 1
fi
[[ "$CLEAN" -eq 1 ]] && rm -rf "$DATA"
exit 0
