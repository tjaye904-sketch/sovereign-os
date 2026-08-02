# Workspace Service — Implementation Specification (v1 minimal)

**Document ID:** SPEC-WORKSPACE-001  
**Status:** Draft for implementation  
**Date:** 2026-07-29

## Purpose

Named thinking environments (domains). Federation of workspaces is later;
v1 is local workspace registry only.

## v1 Scope

- Create / get / list / archive workspaces
- Fields: id, name, kind label, status (Active/Archived), created_at
- File-backed

## Acceptance

1. Create + get + list
2. Archive transitions status
3. Persist across reopen
4. REQUIRE tests under -DNDEBUG
