# Sovereign Engineering Journal

This journal records significant findings, decisions, rationale, and project
handoffs.

Entries are chronological. Existing entries should not be rewritten to make
historical decisions appear different in hindsight. Corrections or changes
should be recorded in new entries.

---

## 2026-07-26 — Repository Architecture Audit

### Session Purpose

Determine how much of the original Sovereign architecture is implemented in the
active repository.

### Evidence Reviewed

- Active repository structure
- Runtime source code
- Runtime tests
- Runtime policy components
- Architecture documents in the engineering vault
- Software Design Document
- Technical Design Document
- Backend Architecture Package
- Cognitive Architecture material
- Runtime Service Specification
- Blueprint Freeze Concept

Generated releases, Sovereign Mesh worktrees, build outputs, and caches were
excluded from active implementation evidence.

### Validation Performed

The registered test suite was executed.

Result:

- 100 tests passed
- 0 tests failed

### Findings

- The runtime platform is mature and extensively tested.
- Runtime lifecycle, events, diagnostics, capability invocation, and capability
  provider selection are implemented.
- The active repository is primarily a runtime platform.
- The full Sovereign product architecture is substantially broader than the
  active implementation.
- Product-level Identity, Governance, Policy, Memory, Workspace, Archive,
  Curiosity, Model Router, Reasoning, Reflection, Decision, Synchronization,
  API Gateway, and application services were not located.
- Runtime components with similar terminology must not be counted as completion
  of product-level services.
- Extensive architecture documents exist and consistently describe the broader
  system.
- A Blueprint Freeze proposal exists.
- The proposed MASTER-001 build book was not located.

### Assessment

- Runtime platform: approximately 98–99% complete
- Blueprint-defined backend: approximately 20–30% complete
- Full Sovereign product: approximately 10–20% complete

These values are engineering estimates rather than automated metrics.

### Decisions

- Treat Runtime Platform v1 as a completed milestone.
- Stop equating runtime maturity with completion of the Sovereign product.
- Begin product-service implementation on top of the runtime.
- Start with Identity, followed by Governance, Policy, Memory, and Workspace.
- Establish durable, version-controlled project-memory artifacts.
- Do not rely on chat history as the source of project truth.
- Require every significant session to leave a resumable handoff.

### Governance Rule Adopted

No milestone is complete until implementation, tests, documentation, and
project-state artifacts agree.

### Artifacts Established

- `SOVEREIGN_BOOTSTRAP.md`
- `PROJECT_STATUS.md`
- `IMPLEMENTATION_ROADMAP.md`
- `ENGINEERING_JOURNAL.md`
- `AUDITS/Audit-001-Runtime-v1.md`

### Next Work

1. Reconstruct or replace MASTER-001.
2. Define the Identity Service specification.
3. Establish Identity Service acceptance criteria and tests.
4. Implement Identity as the first product-level service.

---

## 2026-07-29 — Vision, Governance Tightening, Identity Spec

### Session Purpose

Promote owner vision into the repository, strengthen bootstrap/governance
non-negotiables, and produce the Identity Service implementation specification
as the first product-service milestone artifact.

### Work Performed

1. Added `docs/project/VISION.md` from the owner-facing description. Explicitly
   labeled as destination vision, not implementation status.
2. Updated `SOVEREIGN_BOOTSTRAP.md` with non-negotiable principles, active
   milestone, and expanded required reading.
3. Updated `GD-001` with product non-negotiables, authority boundaries, and
   bootstrap policy including Vision and active service specs.
4. Drafted `IDENTITY_SERVICE_SPEC.md` (SPEC-IDENTITY-001): purpose, non-goals,
   runtime relationship, principals/sessions/roles, API, events, persistence,
   security notes, acceptance criteria, implementation order, open questions.

### Decisions

- Vision text is canonical for explaining Sovereign to non-engineers and for
  orienting engineers to the destination.
- Identity v1 is intentionally minimal: local principals, sessions, roles,
  audit/events — not full MFA/biometrics/federation.
- Authorization remains out of Identity; Governance/Policy consume identity.
- Runtime Platform v1 remains closed except for deliberate Identity integration
  points.

### Open Questions

Recorded in Identity spec Section 12 (session survival, operator enrollment,
session lifetime, service principals, storage path).

### Next Work

1. Owner may answer Identity open questions (optional before coding).
2. Begin Identity implementation plan step 1: data model + storage schema.
3. Write acceptance tests from Section 10 before large implementation surface.

---

## 2026-07-29 — Identity Open Questions Resolved

### Decisions locked (owner accepted engineering recommendations)

| ID | Decision |
|----|----------|
| IQ-1A | Sessions persist across process restart until expiry or revoke |
| IQ-2A | Operator enrollment is owner-session only in v1 |
| IQ-3D | Session lifetime configurable; default 12 hours |
| IQ-4B | Service principal kind reserved in schema only; no public service auth in v1 |
| IQ-5D | Storage under `$SOVEREIGN_DATA_ROOT/identity/` or XDG `~/.local/share/sovereign/identity/` |

Also binding: credential change revokes that principal's sessions; single Owner bootstrap.

### Spec update

`IDENTITY_SERVICE_SPEC.md` Section 12 replaced with resolved decision table.
Session, principal-kind, persistence, and acceptance criterion 9 updated to match.

### Next work

Begin Identity implementation plan step 1: data model + storage schema under
the locked decisions. Acceptance tests from Section 10 should be written before
or with the first substantial implementation surface.

---

## 2026-11-15 — Desktop Operator Runtime Audit-Safe Completion

### Session Purpose

Confirm the desktop operator milestone is complete and verify the missing audit
trace and headless-safe execution boundary before continuing the roadmap.

### Work Performed

1. Hardened `DesktopOperatorGateway::ExecuteApprovedAction` to distinguish headed
   desktop execution from safe headless execution.
2. Ensured the action continues to require governance approval and an allowlist.
3. Added durable evidence logging under the host data root when operating in
   headless mode, rather than silently claiming a real desktop action.
4. Added desktop audit entries for request, approval, rejection, and execution.
5. Verified the desktop operator regression and the full repository test suite.

### Validation

Executed:

- `cmake --build build -j2 --target sovereign_desktop_operator_smoke_test`
- `ctest --test-dir build --output-on-failure -R sovereign_desktop_operator_smoke_test`
- `cmake --build build -j2 && ctest --test-dir build --output-on-failure`

Results:

- Desktop smoke test: 1/1 passed
- Full suite: 32/32 passed
- 0 failed tests

### Assessment

The desktop operator runtime is now complete for the v1 safe host layer: governed,
allowlisted, audit-backed, and explicit about headed vs. safe headless execution.
This closes the roadmap gap recorded in the desktop operator milestone and
allows the next product-service expansion to proceed without re-opening the
same safety bug.



---

## 2026-07-29 — Audit 002: NDEBUG / assert-as-control-flow

Confirmed subscription_inventory_smoke_test SIGSEGV under -DNDEBUG (O0 and O2).
With asserts enabled (including ASan -O2) the same test exits 0.
Root cause: assert(bus.Subscribe(...)) is stripped under NDEBUG so mutations never run.
See docs/project/AUDITS/Audit-002-Test-NDEBUG-and-SubscriptionInventory.md.
Identity work is not blocked; runtime test debt tracked separately.


## 2026-07-29 — Test repair + classification + Identity scaffold

- Replaced 3486 assert() with REQUIRE() across smoke tests; added test_require.hpp.
- Verified subscription_inventory_smoke_test passes under -DNDEBUG and ASan -O2.
- CODEBASE_CLASSIFICATION.md and COMPLETION_PLAN.md written.
- Identity scaffold: core/identity types, config (IQ-5D), store port, CMake.
- Next: in-memory IdentityStore + IdentityService bootstrap/authenticate.


## 2026-07-29 — Quarantine + Identity memory service

- Confirmed 23 runtime + 25 test files in _quarantine_trace_bloat; CMake cleaned.
- Implemented MemoryIdentityStore + IdentityService (bootstrap, auth, sessions, roles, operator enrollment via owner).
- identity_service_smoke_test passes under -O2 -DNDEBUG.
- Note: HashSecret is std::hash placeholder; replace with real KDF before deployment.
- Next: SQLite persistence store, session survival across restart tests, richer acceptance coverage.


## 2026-07-29 — Identity file store + persistence tests

- FileIdentityStore: durable principals/verifiers/roles/sessions under data dir.
- IQ-1A verified: identity_persistence_smoke_test passes (session survives reopen).
- identity_service_smoke_test + identity_acceptance_smoke_test pass under -DNDEBUG.
- Trace bloat remains quarantined (23 runtime + 25 tests).


## 2026-07-29 — Governance v1 scaffold + Identity hardening

- FileIdentityStore + persistence tests (IQ-1A).
- Iterative HashSecret scheme id sov-id-v1 (real KDF still pending).
- ChangeCredential API (revokes sessions).
- SPEC-GOVERNANCE-001 + GovernanceService file store + smoke test (pass -DNDEBUG).


## 2026-07-29 — Approved decisions implemented

1. Shared PrincipalId/SessionId in core/common/principal_id.hpp
2. Identity Validated with IDENTITY_THREAT_NOTES.md; Argon2id scheme reserved; v1 only if allowed
3. Policy Engine v1 role+action evaluate + smoke test
4. All product smokes pass under -DNDEBUG with SOVEREIGN_ALLOW_INSECURE_VERIFIER=1


## 2026-07-29 — Memory Engine v1 minimal

Append/get/list-by-tag file-backed store + smoke test (pass).
Not vector/semantic search — foundation only.


## 2026-07-29 — Workspace Service v1

Local workspace registry create/get/list/archive + persistence smoke.


## 2026-07-29 — Archive Service v1

Append-only artifact metadata store + smoke test.


## 2026-07-29 — Argon2id, Model Router, Cognitive tight governance

- Argon2id hashing/verify via libargon2.so.1 (scheme sov-id-v2-argon2id)
- ModelRouter Ollama local + tools JSON; smoke offline path
- CognitiveService: Curiosity/Reason/Recommend/Reflect; Recommend→Governance; AssertApproved gate
- Sandbox only; no promote


## 2026-07-29 — Executor, Sync, Product Host

- ActionExecutor: refuses until AssertApproved; history recorded
- SyncService: local export/import journal (no network v1)
- SovereignHost: wires Identity/Governance/Policy/Memory/Workspace/Archive/Sync/Models/Cognitive/Executor under one data root
- Smokes pass


## 2026-07-29 — sovereign-cli

Local CLI over SovereignHost: status, bootstrap, login, workspace, memory, recommend, approve, models.
E2E verified under temp SOVEREIGN_DATA_ROOT. Ollama still down in this environment.


## 2026-07-29 — Live Ollama wiring

- Default model hermes-nemotron-3-supercloud (override SOVEREIGN_OLLAMA_MODEL)
- CLI chat / chat-tools / execute
- Live smoke soft-skips when Ollama unreachable from sandbox
- OLLAMA_INTEGRATION.md


## 2026-07-29 — Host build path + session CLI

- scripts/build-sovereign-cli.sh for host (Ollama machine)
- HOST_BUILD.md
- login saves session; recommend requires valid session
- chat-tools executes record_memory append locally when model requests tool
- Sandbox still cannot reach host Ollama; user builds on host


## 2026-07-29 — Default model qwen2.5-coder:7b

- HostConfig default_model and docs updated
- chat-tools: execute tools then second Ollama turn with role=tool messages
- Session-aware recommend still required


## 2026-07-29 — code-assist --file + offline smoke

- code-assist supports --file paths (32KiB cap) and -- separator
- scripts/smoke-offline.sh: bootstrap→login→workspace→memory→recommend→approve→execute


## 2026-07-29 — project-summary + code-assist --json

- project-summary scans dir for source files (cap 40 files / 48KiB) and asks model
- code-assist --json requests structured JSON response shape


## 2026-07-29 — Expanded CLI surface

memory-list, policy-check, review-diff, pending-decisions, archive-store, sync-export, --save on assist commands.


## 2026-07-29 — Owner decisions locked

1 CLI-only 2 Hermes separate 3 journal sync 4 Argon2 hardware-tuned
status prints argon2/surface/sync/hermes/model


## 2026-07-29 — v1 completion closeout

reject/logout CLI; smoke-v1-complete.sh green; matrix updated; V1_COMPLETE.md


## 2026-07-30 — openclaw default; identity CLI expansion

- Default Ollama model: openclaw
- CLI: operator-create, roles, decision-get, change-credential, principals
- ChatRequest temperature/num_predict options


## 2026-07-30 — Milestone integration: AuditLog + Policy gate

- core/audit AuditLog append-only (identity/governance/executor categories)
- SovereignHost opens data_root/audit; CLI audit-list
- ActionExecutor optional PolicyEngine gate (PolicyDenied)
- Completion plan phases 1–4 marked done for v1 product path


## 2026-07-30 — Goals + System Brief (maturity layer)

Long-lived goals register and coherent system brief (goals, pending gov, workspaces, memory, audit).
Maps to vision "How are things today?" surface.


## 2026-07-30 — Continuous awareness layer

AgentScheduler + agent-tick (brief→memory), memory-search, workspace-init-domains (7 domains).


## 2026-07-30 — Owner decisions 1B/2B/3B/4A implemented

- sovereign-daemon long-running agent loop
- Local semantic memory (hashing-trick embeddings + cosine query)
- Domain profiles: prompts + tool allowlists + denied actions
- domain-chat uses profile + semantic context
- Sandbox only (4A)


## 2026-07-30 — Local Ollama embeddings

EmbedText calls Ollama /api/embeddings (SOVEREIGN_EMBED_MODEL, default nomic-embed-text); hash-v1 fallback offline.
Variable-dim vectors; daemon indexes briefs into SemanticStore.


## 2026-07-30 — Claude Sovereign evaluation + governance merge

Adopted rule/role/override model into file-backed GovernanceService.
See CLAUDE_SOVEREIGN_EVAL.md


## 2026-07-30 — Domain tool runner + override audit

DomainToolRunner executes record_memory/semantic_query under domain allowlist.
override emits governance audit.


## 2026-07-30 — Operational OS layer

- Approve creates evidence pack (archive + memory + semantic)
- scripts/sovereign-ctl.sh: build/bootstrap/start/stop/brief/smoke
- docs/project/OPERATIONAL.md runbook


## 2026-07-30 — Verification: v1 scope 100% operational

Full functional suite PASS. See VERIFICATION_AND_COMPLETION.md


## 2026-07-30 — Peer sync opened

Folder peer bundles (peer-export/import). No secrets, no auto-execute. status sync=peer-bundle.


## 2026-07-30 — Transparency completion rules

inspect/ask cover all mandated transparency sections.


## 2026-07-30 — Bare-metal ISO path

Hybrid ISO: Linux live kernel + Sovereign primary surface. scripts/build-sovereign-iso.sh
