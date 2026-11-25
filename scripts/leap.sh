#!/usr/bin/env bash
# leap.sh: isolated agent sessions + one branch per L packet; main only receives green L-packet merges.
#
#   scripts/leap.sh status                              where things stand (active packets, agent sessions, results)
#   scripts/leap.sh agent-start <agent> <topic> [base]  new agent session branch (defaults to main)
#   scripts/leap.sh start leap/L7-name [base]           new L-packet branch (defaults to main)
#   scripts/leap.sh check                               build + full ctest + main.cpp guard on current branch
#   scripts/leap.sh finish                              check, then merge leap/* into main only if 100% green
#
# Env: MAIN=main (default), TEST_CMD="..." to override the build+test command.
set -euo pipefail

MAIN="${MAIN:-main}"
cd "$(git rev-parse --show-toplevel)"
STATE="$(git rev-parse --git-dir)/leap-status"
mkdir -p "$STATE"

TRAILER_1="Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>"
TRAILER_2="Claude-Session: https://claude.ai"

die() { echo "ERROR: $*" >&2; exit 1; }
cur() { git rev-parse --abbrev-ref HEAD; }
slug() { echo "${1//\//__}"; }
clean() { [ -z "$(git status --porcelain)" ]; }

run_tests() {
  # NOTE: callers use this inside `if`, where `set -e` is disabled, so every
  # step must fail explicitly with `|| return 1`.
  if [ -f "apps/cli/main.cpp" ] && grep -q '<< "        << "' apps/cli/main.cpp; then
    echo "MANGLED apps/cli/main.cpp DETECTED"; return 1
  fi
  if [ -n "${TEST_CMD:-}" ]; then bash -c "$TEST_CMD"; return; fi
  local log; log="$(mktemp)"
  cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DSOVEREIGN_BUILD_TESTS=ON >/dev/null || { echo "CONFIGURE FAILED"; return 1; }
  cmake --build build -j"$(nproc)" || { echo "BUILD FAILED"; return 1; }
  ctest --test-dir build --output-on-failure 2>&1 | tee "$log"
  [ "${PIPESTATUS[0]}" -eq 0 ] || { echo "CTEST FAILED"; return 1; }
  grep -q "100% tests passed, 0 tests failed" "$log"
}

record() { # branch result
  local n; n="$(ctest --test-dir build -N 2>/dev/null | sed -n "s/^Total Tests: //p" || true)"
  echo "$2 tests=${n:-?} sha=$(git rev-parse --short HEAD) at=$(date '+%F %T')" > "$STATE/$(slug "$1")"
}

print_branch_group() {
  local label="$1" pattern="$2" b state merged
  echo "$label:"
  for b in $(git for-each-ref --format='%(refname:short)' "$pattern"); do
    if git merge-base --is-ancestor "$b" "$MAIN"; then
      merged="MERGED"
    else
      merged="ACTIVE (+$(git rev-list --count "$MAIN..$b") commits not in $MAIN)"
    fi
    state="$(cat "$STATE/$(slug "$b")" 2>/dev/null || echo 'no recorded test run')"
    printf '  %-50s %s\n      last check: %s\n' "$b" "$merged" "$state"
  done
}

cmd_status() {
  echo "current branch : $(cur)$(clean || echo '   (UNCOMMITTED CHANGES)')"
  echo "main           : $(git rev-parse --short "$MAIN") $(git log -1 --format=%s "$MAIN")"
  echo
  print_branch_group "leap branches" "refs/heads/leap/*"
  echo
  print_branch_group "agent session branches" "refs/heads/agent/*"
}

cmd_agent_start() {
  local agent="${1:-}" topic="${2:-}" base="${3:-$MAIN}"
  [ -n "$agent" ] && [ -n "$topic" ] || die "usage: leap.sh agent-start <agent-name> <short-topic> [base-ref]"
  clean || die "working tree not clean"
  local ts b
  ts="$(date '+%Y%m%d-%H%M')"
  b="agent/${agent}/${ts}-${topic}"
  git show-ref --verify --quiet "refs/heads/$b" && die "$b already exists"
  git checkout "$base"
  git checkout -b "$b"
  echo "Active agent session: $b (cut from $base @ $(git rev-parse --short HEAD))"
}

cmd_start() {
  local b="${1:-}" base="${2:-$MAIN}"
  [[ "$b" == leap/* ]] || die "usage: leap.sh start leap/L7-name [base-ref]"
  clean || die "working tree not clean"
  git show-ref --verify --quiet "refs/heads/$b" && die "$b already exists"
  git checkout "$base"
  git checkout -b "$b"
  echo "Active packet: $b (cut from $base @ $(git rev-parse --short HEAD))"
}

cmd_check() {
  local b; b="$(cur)"
  [[ "$b" == leap/* || "$b" == agent/* ]] || die "must be on a leap/* or agent/* branch (currently on $b)"
  clean || die "working tree not clean: commit first so the result maps to a SHA"
  if run_tests; then record "$b" PASS; echo "PASS recorded for $b"; else record "$b" FAIL; die "tests failed on $b"; fi
}

cmd_finish() {
  local b; b="$(cur)"
  [[ "$b" == leap/* ]] || die "only leap/* branches can merge to $MAIN (agent/* branches must promote into a leap/* branch first)"
  cmd_check
  git checkout "$MAIN"
  # Merge without committing, test the exact merged tree, commit only if green.
  if ! git merge --no-ff --no-commit "$b"; then
    git merge --abort || true; git checkout "$b"; die "merge conflict: main untouched"
  fi
  if run_tests; then
    git commit -q -m "merge $b (all tests green)" -m "$TRAILER_1" -m "$TRAILER_2"
    git tag -f "${b//\//-}-merged" >/dev/null
    record "$b" "MERGED-PASS"
    echo "Merged $b into $MAIN at $(git rev-parse --short HEAD). Next: scripts/leap.sh start leap/L<next>-<name>"
  else
    git merge --abort || git reset --hard HEAD
    git checkout "$b"
    die "merged tree failed tests: $MAIN untouched"
  fi
}

case "${1:-status}" in
  status)      cmd_status ;;
  agent-start) shift; cmd_agent_start "$@" ;;
  start)       shift; cmd_start "$@" ;;
  check)       cmd_check ;;
  finish)      cmd_finish ;;
  *) die "unknown command: $1" ;;
esac
