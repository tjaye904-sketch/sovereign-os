# Leap log (append-only)

## DECISIONS
- 2026-11-17: Adopt MASTER_BUILD_PLAN — workspace↔headless node; Sovereign Jarvis across federation; Core persists beyond single node; dual profile; sequential leaps only.

## COMPLETED
- L0.1 Master plan docs in repo (MASTER_BUILD_PLAN.md)
- L0.2 COMPONENT_REGISTRY.yaml live; 16 workspace names registered; domain list helper
- L0.3 PROFILES.md host-pop / nixos-sovereign

## IN_PROGRESS
- none

## NEXT
- L1.1 Secret Manager MVP (do not start node fabric until L1.1–L1.3 per plan preference)

- L1.2 Memory facets (tags + CLI + inspect counts)

## NEXT
- L1.3 Workspace Controller MVP

- 2026-11-18: Merged origin orchestration/cognitive; fixed api_gateway link; 41/41 CTest; editor state untracked

- L1.4 Capability registry (seed/list/enable/disable/check)

## NEXT
- L2.1 Node registry (or product hardening as prioritized)

## Branch policy
- One leap per branch `leap/L{N}-…`
- Merge to main only at 100% CTest

## Completed
- L1 capability registry
- L2 node federation
- L3 CapabilityGate + gate-check (e87a8f7)

## In progress
- L4 capability bridge (Python CU/sys)
