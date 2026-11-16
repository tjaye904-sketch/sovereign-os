#!/usr/bin/env bash
# Gap 24
set -euo pipefail
export PATH="$HOME/.local/bin:$PATH"
ROOT="${1:-$HOME/src/sovereign-os/sovereign-work}"
[[ -f "$HOME/.config/sovereign/env.bash" ]] && source "$HOME/.config/sovereign/env.bash" || true
OUT="$ROOT/docs/os2.0/SPEC_COVERAGE_HOST.md"
{
  echo "# Specification coverage (host snapshot)"
  echo
  echo "Generated: $(date -Is)"
  echo
  echo '```'
  sovereign-cli inspect 2>/dev/null | head -120 || echo "(inspect unavailable)"
  echo '```'
  echo
  echo "## Automated tests"
  echo "- CTest product suite: run \`ctest --test-dir build\`"
  echo "- E2E: \`sovereign-e2e-host\`"
} > "$OUT"
echo "wrote $OUT"
