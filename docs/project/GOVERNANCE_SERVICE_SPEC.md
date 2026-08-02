# Governance Service — Implementation Specification

**Document ID:** SPEC-GOVERNANCE-001  
**Status:** Draft for implementation  
**Date:** 2026-07-29  
**Depends on:** Identity Service (sessions / principals)  
**Milestone:** 2.2

## Purpose

Governance records **approval requirements and decisions** for consequential
actions. It consumes Identity sessions; it does not authenticate users.

## v1 Scope

- Decision records (request, evidence refs, status, approver)
- Require owner session for approve/reject
- Pending / approved / rejected / cancelled states
- Durable file-backed store (same data-root convention as Identity)

## Non-Goals v1

- Full policy language evaluation (Policy Engine milestone)
- Multi-party quorum
- UI

## Acceptance (v1)

1. Create decision request bound to requester session principal
2. List pending decisions
3. Owner session can approve or reject with reason
4. Non-owner cannot approve
5. Persistence across process restart
6. REQUIRE-based automated tests pass under -DNDEBUG
