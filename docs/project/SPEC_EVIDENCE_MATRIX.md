# Specification Evidence Matrix

| Capability | Spec / decision | Evidence |
|------------|-----------------|----------|
| Identity Argon2id | IDENTITY_SERVICE_SPEC | identity_*_smoke_test; cli bootstrap/login |
| Governance rules/override | GOVERNANCE + Claude merge | governance_service_smoke_test; cli approve/override |
| Policy role+action | POLICY_ENGINE_SPEC | policy_engine_smoke_test; cli policy-check |
| Memory append/tag | MEMORY_ENGINE_SPEC | memory_engine_smoke_test |
| Workspace domains | WORKSPACE_SERVICE_SPEC | workspace_service_smoke_test; workspace-init-domains |
| Archive evidence | product | archive_service_smoke_test; approve evidence_artifact |
| Model router Ollama | MODEL_ROUTER_SPEC | model_router_smoke_test |
| Cognitive tight gov | COGNITIVE_SERVICES_SPEC | cognitive_governance_smoke_test |
| Executor approved-only | product | action_executor_smoke_test |
| Peer bundle sync | OWNER peer decision | peer-export/import e2e |
| Goals + briefing | product | cli goal-*; brief |
| Semantic local embed | OWNER 2B | semantic-add/query |
| Domain agents | OWNER 3B | domain-show/chat |
| Daemon continuous | OWNER 1B | sovereign-daemon |
| Transparency rules | user list | inspect/ask |
| Runtime substrate | IMPLEMENTATION_ROADMAP | runtime foundational ctests |
| Clean ZIP CI | quality gate | .github/workflows/clean-zip-build.yml |
| E2E temp data root | quality gate | scripts/e2e-temp-dataroot.sh |

Deferred by owner: HTTP API, Desktop, Hermes bridge.
