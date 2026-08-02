#!/usr/bin/env bash
# Build sovereign-cli on the host machine (where Ollama runs).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-$ROOT/build/sovereign-cli}"
mkdir -p "$(dirname "$OUT")"

CXX="${CXX:-g++}"
STD="${STD:--std=c++23}"
OPT="${OPT:--O2 -DNDEBUG}"
INC=(
  -I core/common -I core/audit -I core/goals -I core/briefing -I core/agent -I core/semantic -I core/domain -I core/identity -I core/governance -I core/policy
  -I core/memory -I core/workspace -I core/archive -I core/model_router
  -I core/cognitive -I core/executor -I core/sync -I core/product -I core/diagnostics
)

SRCS=(
  core/audit/audit_log.cpp
  core/goals/goals_service.cpp
  core/briefing/briefing_service.cpp
  core/agent/agent_scheduler.cpp
  core/semantic/local_embedder.cpp
  core/semantic/semantic_store.cpp
  core/domain/domain_profile.cpp
  core/domain/domain_tool_runner.cpp
  core/identity/identity_config.cpp
  core/identity/identity_verifier.cpp
  core/identity/identity_store_file.cpp
  core/identity/identity_service.cpp
  core/governance/governance_service.cpp
  core/memory/memory_engine.cpp
  core/policy/policy_engine.cpp
  core/workspace/workspace_service.cpp
  core/archive/archive_service.cpp
  core/sync/sync_service.cpp
  core/sync/peer_bundle.cpp
  core/model_router/ollama_client.cpp
  core/model_router/model_router.cpp
  core/cognitive/cognitive_service.cpp
  core/executor/action_executor.cpp
  core/product/sovereign_host.cpp
  core/diagnostics/transparency_report.cpp
  apps/cli/main.cpp
)

ARGON_LIB=""
if [[ -f /usr/lib/x86_64-linux-gnu/libargon2.so.1 ]]; then
  ARGON_LIB=/usr/lib/x86_64-linux-gnu/libargon2.so.1
elif [[ -f /usr/lib/libargon2.so ]]; then
  ARGON_LIB=/usr/lib/libargon2.so
else
  ARGON_LIB=-largon2
fi

echo "Compiling sovereign-cli -> $OUT"
# shellcheck disable=SC2086
$CXX $STD $OPT "${INC[@]}" "${SRCS[@]}" -o "$OUT" $ARGON_LIB -lpthread
echo "OK: $OUT"
echo "Try:"
echo "  export OLLAMA_HOST=http://127.0.0.1:11434"
echo "  export SOVEREIGN_OLLAMA_MODEL=\$(ollama list | awk 'NR==2{print \$1}')"
echo "  $OUT models"
echo "  $OUT chat 'Reply with PONG'"

# Also build daemon
DAEMON_OUT="$(dirname "$OUT")/sovereign-daemon"
echo "Compiling sovereign-daemon -> $DAEMON_OUT"
$CXX $STD $OPT "${INC[@]}" \
  core/audit/audit_log.cpp core/goals/goals_service.cpp core/briefing/briefing_service.cpp \
  core/agent/agent_scheduler.cpp core/semantic/local_embedder.cpp core/semantic/semantic_store.cpp \
  core/domain/domain_profile.cpp
  core/domain/domain_tool_runner.cpp \
  core/identity/identity_config.cpp core/identity/identity_verifier.cpp \
  core/identity/identity_store_file.cpp core/identity/identity_service.cpp \
  core/governance/governance_service.cpp core/memory/memory_engine.cpp \
  core/policy/policy_engine.cpp core/workspace/workspace_service.cpp \
  core/archive/archive_service.cpp core/sync/sync_service.cpp
  core/sync/peer_bundle.cpp \
  core/model_router/ollama_client.cpp core/model_router/model_router.cpp \
  core/cognitive/cognitive_service.cpp core/executor/action_executor.cpp \
  core/product/sovereign_host.cpp
  core/diagnostics/transparency_report.cpp apps/daemon/main.cpp \
  -o "$DAEMON_OUT" $ARGON_LIB -lpthread
echo "OK: $DAEMON_OUT"
