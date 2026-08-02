# Sovereign Implementation Roadmap

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

**Status:** Next phase

### Milestone 2.1 — Identity Service

**Status:** Not started

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

**Status:** Not started

Target responsibilities:

- Human authority boundaries
- Governance rules
- Approval requirements
- Decision accountability
- Governance event recording
- Emergency and override behavior

### Milestone 2.3 — Policy Engine

**Status:** Not started

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

**Status:** Not started

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

**Status:** Not started

Target responsibilities:

- Workspace creation and lifecycle
- Membership
- Context boundaries
- Resource ownership
- Memory scoping
- Governance scoping
- API surface

## Phase 3 — Knowledge and Integration Services

**Status:** Planned

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

**Status:** Planned

Recommended order:

1. Reasoning Engine
2. Decision Engine
3. Reflection Engine
4. Planning and orchestration

These services must depend on governed identity, memory, policy, and workspace
boundaries rather than bypassing them.

## Phase 5 — Interfaces and Applications

**Status:** Planned

Potential deliverables:

- API Gateway
- Administrative interface
- Desktop application
- Command-line interface
- Developer SDK
- Integration adapters
- Observability interface

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
