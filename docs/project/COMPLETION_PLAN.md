# Sovereign Completion Plan (sandbox execution)

**Authority:** GD-001, VISION.md, IDENTITY_SERVICE_SPEC.md, Audit 001–002  
**Work root:** `/home/workdir/artifacts/sovereign-work` only

## Phase 0 — Hygiene (done / in progress)

- [x] Sandbox policy; original tree read-only
- [x] Vision, Bootstrap, GD-001, Identity spec, IQ decisions
- [x] Audit 002 NDEBUG / assert-as-control-flow
- [x] Replace all smoke-test `assert(` with always-on `REQUIRE(`
- [x] Codebase classification (substrate vs bloat vs missing product)
- [ ] Optional: quarantine trace-migration stub headers (non-blocking)

## Phase 1 — Identity Service v1 (ACTIVE)

Per SPEC-IDENTITY-001 and locked IQ-1A…IQ-5D:

1. [x] Types + config + store port
2. [x] In-memory store + durable FileIdentityStore (persistence; SQLite optional later)
3. [x] IdentityService API (bootstrap, auth, sessions, roles)
4. [x] Credential verifier (Argon2id hardware-tuned)
5. [x] Structured AuditLog (product); runtime event bus integration still optional
6. [x] Smoke tests with REQUIRE (service, persistence, acceptance)
7. [x] Threat notes + matrix/journal update → DoD

## Phase 2 — Governance Service

- [x] Decision request/approve/reject/list + file store
- [x] Cognitive Recommend → Pending; Executor AssertApproved
- [x] Audit emits on approve/execute (CLI)

## Phase 3 — Policy, Memory, Workspace

- [x] Policy role+action; gated on Executor when roles supplied
- [x] Memory / Workspace / Archive file stores + CLI

## Phase 4 — Model Router, Cognitive, Product Host, CLI

- [x] Ollama Model Router (default openclaw)
- [x] Cognitive tight governance
- [x] SovereignHost + sovereign-cli + smoke-v1-complete

## Phase 5 — Integration hardening (active)

- [x] Product AuditLog
- [x] Policy gate on ActionExecutor
- [ ] Optional runtime EventBus adapter for audit (non-blocking)
- [ ] AssignRole / SetPrincipalStatus CLI


## Explicit non-work

- Do not extend capability-trace migration name chains
- Do not claim product completion from runtime primitives
- Do not modify original sovereign archive

## Phase 1b — CLI surface (done in sandbox)

- [x] sovereign-cli build script for host
- [x] Session-aware recommend
- [x] Gated execute
- [x] Memory/policy/archive/sync commands
- [x] code-assist / project-summary / review-diff (need Ollama on host)

## Still needs owner input later

- Multi-device peer sync protocol and threat model
- UI / Hermes dashboard bridge design
- Production Argon2 cost tuning on target hardware
- Whether to expose a local HTTP API vs CLI-only for v1
