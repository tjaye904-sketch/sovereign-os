# Policy Engine — Implementation Specification

**Document ID:** SPEC-POLICY-001  
**Status:** Draft for implementation  
**Date:** 2026-07-29  
**Depends on:** Identity (roles/sessions), Governance (optional approval hooks)

## Purpose

Evaluate whether an authenticated principal may perform an action under local rules.

## v1 Scope

- Allow/deny by role + action string
- Built-in defaults: owner allow-all; operator allow subset; readonly deny mutations
- Does not replace Governance approvals for consequential actions — Policy may require `governance_approved=true` evidence flag later

## Non-Goals v1

- Full policy language / DSL
- Remote policy bundles

## Acceptance

1. Owner principal allowed for action "configure"
2. Readonly denied for action "deploy"
3. Unknown principal denied
4. REQUIRE tests under -DNDEBUG
