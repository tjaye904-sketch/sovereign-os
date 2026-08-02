# Sovereign Bootstrap

## Purpose

This document provides the minimum context required for an engineer or AI
assistant to begin productive work on the Sovereign project.

## What Sovereign Is

Sovereign is a local-first, human-governed cognitive operating system whose
identity is independent of any particular AI model.

The accessible owner-facing vision is recorded in `VISION.md`.

The architecture separates:

- Runtime infrastructure
- Governance
- Identity
- Memory
- Workspace management
- Cognitive services
- AI model providers

AI models are replaceable implementation details.

### Non-negotiable principles

1. **Human authority is final** over consequential actions.
2. **Local-first** operation and owner control of primary data.
3. **Model independence** — no model defines Sovereign's identity.
4. **Traceability** — important decisions leave reviewable evidence.
5. **Repository artifacts are the source of truth** — conversation is not.
6. **Runtime primitives are not product services** — similar names must not be
   treated as completion of Identity, Memory, Governance, Policy, or Model Router.
7. **No silent failure** — errors must be observable and attributable.
8. **Vertical slice before isolated expansion** — integrate into a working path
   rather than accumulating disconnected modules.

## Current Repository Status

The active repository primarily implements the **Sovereign Runtime Platform**.

Major implemented areas include:

- Runtime lifecycle
- Event infrastructure
- Scheduling
- Diagnostics
- Capability invocation
- Capability provider selection
- Runtime policy composition
- Testing framework

Runtime validation (Audit-001):

- All 100 registered runtime tests pass.

## What Is Not Yet Implemented

The following blueprint-defined services are not present as active product-level
implementations:

- Identity Service
- Governance Service
- Policy Engine
- Memory Engine
- Workspace Service
- Archive Service
- Curiosity Engine
- Model Router
- Reasoning Engine
- Reflection Engine
- Decision Engine
- Synchronization Service
- API Gateway
- Product applications
- Federation and domain workspaces

Runtime primitives with similar names must not be mistaken for these services.

Examples:

- `identity_types.hpp` provides runtime identity primitives; it is not the
  Identity Service.
- `memory_log_sink` is a logging component; it is not the long-term Memory
  Engine.
- Runtime capability provider selection is not the complete Model Router.

## Architecture References

Primary design references reside in the Sovereign engineering vault and in
promoted project documents under `docs/project/`.

Major references include:

- `VISION.md` (canonical vision)
- Sovereign AI Software Design Document
- Sovereign Technical Design Document
- Backend Architecture Package
- Architecture Decision Records
- Runtime Service Specification
- Sovereign AI Engineering Specifications
- Cognitive Architecture documents
- Blueprint Freeze Concept

The proposed `MASTER-001 Sovereign Master Build Book` was not located during
Audit 001. Service specifications and the implementation roadmap currently
serve as the practical build sequence.

## Current Development Phase

**Completed phase:** Runtime Platform v1

**Next phase:** Core Backend Service Implementation

**Active milestone:** Identity Service

Recommended implementation order:

1. Identity
2. Governance
3. Policy
4. Memory
5. Workspace
6. Archive
7. Curiosity
8. Model Router
9. Higher cognitive services
10. Applications and integrations

## Required Reading

Before beginning engineering work:

1. Read `VISION.md` (destination, not status).
2. Read `PROJECT_STATUS.md`.
3. Read the latest report in `AUDITS/`.
4. Read `IMPLEMENTATION_MATRIX.md`.
5. Read `IMPLEMENTATION_ROADMAP.md`.
6. Review the latest entry in `ENGINEERING_JOURNAL.md`.
7. Review `governance/GD-001-Engineering-Governance.md`.

For Identity work, also read `IDENTITY_SERVICE_SPEC.md` when present.

## Operating Rule

Do not rely on conversational memory as the authoritative source of project
state.

Repository artifacts are the source of truth.

Every significant engineering session should leave durable updates to status,
roadmap, matrix, and journal as required by GD-001.
