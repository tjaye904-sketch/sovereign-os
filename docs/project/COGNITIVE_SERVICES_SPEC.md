# Cognitive Services — Implementation Specification

**Document ID:** SPEC-COGNITIVE-001  
**Status:** Draft for implementation  
**Date:** 2026-07-29  
**Owner decision:** Curiosity / Reasoning / Decision / Reflection bound **very tightly** to Governance.

## Principle

No consequential side effect may proceed from a cognitive recommendation without a
Governance Decision in Approved state. Cognitive modules produce *proposals* and
*analyses*; Governance produces *authority*.

## Modules (v1)

| Module | Role |
|--------|------|
| Curiosity | Propose investigation questions / topics (no external action) |
| Reasoning | Structure arguments for/against a question |
| Decision | Package a recommended action as a Governance decision request |
| Reflection | Post-hoc review notes stored to Memory (optional) |

## Tight coupling rules

1. `DecisionService::Recommend` creates a **Pending** Governance decision; it does not execute.
2. Callers must pass `governance_decision_id` + Approved status before any executor runs.
3. Curiosity/Reasoning outputs are non-authoritative text artifacts.
4. Reflection may append Memory records only; no system mutation.

## Non-Goals v1

- Full multi-agent debate
- Automatic tool execution without Governance
