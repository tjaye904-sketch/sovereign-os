# Sovereign Implementation Roadmap

> **STATUS NOTE (2026-11):** Host operator path (inventory, CU/sys/hub, session) and C++ identity/governance/policy/memory/workspace/cognitive engines exist in-repo. Phase labels below distinguish *v1 code present* vs *product-depth / formal specs still open*.

## Roadmap Principle

The runtime platform is the substrate.

Future milestones must implement the product-level services defined by the
Sovereign architecture rather than duplicating or indefinitely extending the
runtime layer.

## Phase 1 — Runtime Platform

**Status:** Complete

### Delivered

- Runtime lifecycle
- Event infrastructure
- Event routing
- Retry policy
- Scheduling
- Diagnostics
- Capability invocation
- Capability provider selection
- Runtime policy composition
- Runtime identity primitives
- Test infrastructure

### Exit Evidence

- 100 registered tests passed.
- Runtime source and test structure were audited.
- Runtime scope was distinguished from product-service scope.

## Phase 2 — Core Platform Services

**Status:** v1 complete in tree — deepen (specs, threat models, formal schema)

### Milestone 2.1 — Identity Service

**Status:** v1 complete (principals, sessions, MFA scaffold, revocation); deepen remaining

Target responsibilities:

- Persistent Sovereign identity
- Identity lifecycle
- Identity metadata
- Identity verification boundaries
- Runtime integration
- Governance integration
- Auditable identity changes

Required outputs:

- Service specification
- Public interfaces
- Persistence model
- Runtime adapter
- Unit tests
- Integration tests
- Threat model
- Project-state update

### Milestone 2.2 — Governance Service

**Status:** v1 complete (request/approve/reject/override + audit); deepen remaining

Target responsibilities:

- Human authority boundaries
- Governance rules
- Approval requirements
- Decision accountability
- Governance event recording
- Emergency and override behavior

### Milestone 2.3 — Policy Engine

**Status:** v1 complete (versioning, dry-run, rule-set); deepen remaining

Target responsibilities:

- Product-level policy evaluation
- Policy definitions and versioning
- Context-sensitive authorization
- Explainable policy decisions
- Governance integration
- Audit records

Runtime capability policies may be reused as infrastructure, but they do not
constitute the complete product Policy Engine.

### Milestone 2.4 — Memory Engine

**Status:** v1 complete (append, search, rank, prune, provenance tags); deepen remaining

Target responsibilities:

- Durable long-term memory
- Memory types and schemas
- Provenance
- Retrieval
- Retention
- Correction and deletion
- Access policy
- Identity and workspace boundaries

A runtime log sink must not be treated as completion of this milestone.

### Milestone 2.5 — Workspace Service

**Status:** v1 complete (create/list/domain init); deepen remaining

Target responsibilities:

- Workspace creation and lifecycle
- Membership
- Context boundaries
- Resource ownership
- Memory scoping
- Governance scoping
- API surface

## Phase 3 — Knowledge and Integration Services

**Status:** Partial — archive/sync/model_router present; curiosity engine present in cognitive layer

Recommended order:

1. Archive Service
2. Curiosity Engine
3. Model Router
4. Synchronization Service

### Archive Service

Provides durable archival storage, provenance, retrieval, retention, and
governance controls.

### Curiosity Engine

Produces governed questions, information gaps, research proposals, and
exploration tasks.

### Model Router

Selects models and providers using capability, policy, privacy, availability,
cost, and workload requirements.

Runtime capability-provider selection may serve as a lower-level dependency.

### Synchronization Service

Coordinates approved synchronization across devices or nodes while preserving
local-first ownership and governance.

## Phase 4 — Cognitive Services

**Status:** v1 in tree (reasoning, decision, reflection, planning, orchestration, evolution sandbox) — wire honest model enrichment and product depth

Recommended order:

1. Reasoning Engine
2. Decision Engine
3. Reflection Engine
4. Planning and orchestration

These services must depend on governed identity, memory, policy, and workspace
boundaries rather than bypassing them.

## Phase 5 — Interfaces and Applications

**Status:** Mostly complete in current repo state

Current repo evidence:

- CLI surface is implemented and validated
- Product host composition is in place
- Governance-aware execution and memory access are active
- Model router and cognitive services are wired in

Potential deliverables:

- API Gateway
- Administrative interface
- Desktop application
- Command-line interface
- Developer SDK
- Integration adapters
- Observability interface

## Phase 6 — Safe Autonomous Self-Evolution

**Status:** Complete / active safety layer

This phase extends the project from "observe and propose" into a governed, sandboxed improvement loop without live mutation of the canonical repo.

### Required deliverables

1. Isolated proposal generation for subsystem-level improvements.
2. Dedicated sandbox clone of the active repository tree.
3. Build and test validation entirely within the sandbox.
4. Proposal evidence package containing summary, branch/worktree context, and validation logs.
5. Human approval gate before any live integration occurs.
6. Explicit rejection path with clear documented reasons.
7. No mutation of the canonical tree unless a human authorizes the merge.

### Evidence of completion

- Proposal generation and validation pass in the sandbox.
- Proof that the live repo remains unchanged during proposal creation.
- Human approval required before ApplyApprovedProposal or integration.
- Worktree or sandbox policy remains mandatory for all consequential edits.

## Phase 7 — External Interface Authorization

**Status:** Implemented for the v1 local product surface

This phase defines the external boundary where requests from authenticated principals are transformed into governance decisions before execution.

### Required deliverables

1. External request object representing a user or system request.
2. Principal-bound authentication before any external request is accepted.
3. Governance decision generation for all external actions.
4. Approval and rejection flow tied to the product governance service.
5. No direct execution path that bypasses the approval gate.

### Evidence of completion

- External requests can be generated and approved through the host layer.
- Decision IDs are recorded and validated with approval gating.
- The CLI exposes the request/approve flow for external users.

## Phase 8 — Desktop Operator Runtime

**Status:** Complete

This phase formalizes the AI-as-admin desktop model by creating a governed desktop-action boundary. The operator may request host actions such as snapshotting, focus, or app launch, but those actions remain gated behind approval, policy awareness, and audit-backed intent.

### Required deliverables

1. Desktop action request flow bound to identity and governance. — Complete
2. Approval and rejection semantics for visible host-level actions. — Complete
3. Safe allowlist for desktop actions that are valid under the operating model. — Complete
4. Clear evidence packaging for desktop-intent execution. — Complete
5. No direct host action path that bypasses governance approval. — Complete

### Evidence of completion

- Desktop requests can be created and approved through the host service layer.
- Approved action decisions are validated before execution is considered eligible.
- Action intent remains constrained to the operator-safe action set.
- Headless and headed execution paths differ explicitly and write durable desktop evidence.
- The desktop operator smoke test verifies the audit trail under the `desktop` category.

## Milestone Definition of Done

No milestone is complete until all applicable requirements are satisfied:

- Implementation is complete.
- Automated tests pass.
- Integration behavior is validated.
- Security and governance implications are reviewed.
- Public interfaces are documented.
- Architecture documentation matches the implementation.
- `PROJECT_STATUS.md` is updated.
- `IMPLEMENTATION_ROADMAP.md` is updated.
- `ENGINEERING_JOURNAL.md` is updated.
- A formal audit is added when appropriate.
- The repository can be resumed by a new engineer or AI without substantial
  rediscovery.

## Immediate Next Actions

1. Reconstruct or formally replace the proposed MASTER-001 build book.
2. Produce the Identity Service implementation specification.
3. Define Identity Service acceptance tests before implementation.
4. Map runtime facilities that Identity may reuse.
5. Begin Identity Service implementation as the first product-service milestone.
