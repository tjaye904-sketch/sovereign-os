# Codebase Classification (sovereign-work)

**Date:** 2026-07-29  
**Scope:** Active sandbox tree only

## Testing

| Category | Assessment |
|----------|------------|
| Smoke tests (100) | **Useful after repair** — all `assert(` replaced with always-on `REQUIRE(` via `test_require.hpp` |
| Prior NDEBUG behavior | **Broken** — fixed for assert-as-control-flow (Audit 002) |

## core/runtime

### Complete / useful substrate (keep)

Lifecycle, events, routing, dead-letter, subscriptions, scheduler, service registry/host, config, logging, health, UUID, message bus, service invoke/dispatch/endpoint, capability negotiate/invoke, provider selection policies, identity **primitives** (IDs only).

### Useful but secondary

Capability pipeline diagnostics, trace **query** helpers that serve real operational needs (keep lean subset).

### Bloat / combinatorial noise (quarantine — do not extend)

~28 headers/sources with extreme names under capability-trace migration/dispatch/diagnostic/recovery/verification/continuation/stability chains. Many are stub `available()` types. **Do not treat as product progress.** Prefer freeze; optional later delete or archive folder.

### Incomplete vs product architecture

No product Identity/Governance/Policy/Memory/Workspace services in `core/`. Runtime identity types ≠ Identity Service.

## docs/project

Canonical vision, bootstrap, GD-001, status, matrix, roadmap, journal, audits, Identity spec — **authoritative**.

## Plan alignment

Runtime = closed platform. Product path starts at Identity Service per SPEC-IDENTITY-001.
