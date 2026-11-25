#!/usr/bin/env bash
# L7 apply: wires core/federation + peer_auth smoke test into the build. Idempotent.
set -euo pipefail
cd "$(dirname "$0")/.."

ROOT_CM=CMakeLists.txt
TEST_CM=testing/CMakeLists.txt

[ -f "$ROOT_CM" ] && [ -f "$TEST_CM" ] || { echo "run from the sovereign-work repo"; exit 1; }
[ -d core/node ] || { echo "core/node missing: is L2 merged on this branch?"; exit 1; }

if ! grep -q 'add_subdirectory(core/federation)' "$ROOT_CM"; then
  grep -q 'add_subdirectory(core/node)' "$ROOT_CM" || { echo "anchor add_subdirectory(core/node) not found"; exit 1; }
  sed -i 's|^add_subdirectory(core/node)$|add_subdirectory(core/node)\nadd_subdirectory(core/federation)|' "$ROOT_CM"
  echo "wired core/federation into $ROOT_CM"
fi

if ! grep -q 'peer_auth_smoke_test' "$TEST_CM"; then
  printf '\nsovereign_add_smoke(peer_auth_smoke_test federation sovereign_federation)\n' >> "$TEST_CM"
  echo "registered peer_auth_smoke_test in $TEST_CM"
fi
echo "L7 applied"
