# Memory Engine — Implementation Specification (v1 minimal)

**Document ID:** SPEC-MEMORY-001  
**Status:** Draft for implementation  
**Date:** 2026-07-29  
**Depends on:** Identity (optional principal attribution)

## Purpose

Durable local structured notes (not LLM chat history). Append-oriented records
with tags for later retrieval. Foundation for long-term Memory in the vision.

## v1 Scope

- Append MemoryRecord (id, timestamp, principal optional, tags, body)
- List by tag
- Get by id
- File-backed store under data root / memory/

## Non-Goals v1

- Embedding / vector search
- Cross-workspace federation
- Automatic summarization

## Acceptance

1. Append + get round-trip
2. List by tag returns matches
3. Persistence across reopen
4. REQUIRE tests under -DNDEBUG
