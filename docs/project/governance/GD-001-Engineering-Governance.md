# GD-001 — Sovereign Engineering Governance

Status: Accepted

Date: 2026-07-26

Updated: 2026-07-29

---

## Purpose

Define the engineering governance rules used to develop Sovereign.

---

## Principles

### Repository First

Repository artifacts are the authoritative source of project knowledge.

Conversation history is not.

---

### Durable Knowledge

Engineering decisions must be preserved in version-controlled documents.

---

### Session Handoff

Every significant engineering session should conclude with:

- Updated project status
- Updated roadmap
- Updated journal
- Updated implementation matrix
- Audit (when appropriate)
- Clearly identified next work item

---

### Definition of Done

A milestone is complete only when:

- Implementation complete
- Tests passing
- Documentation updated
- Architecture synchronized
- Project Status updated
- Roadmap updated
- Implementation Matrix updated
- Journal updated

---

### Audit Policy

Audits are historical records.

They are never rewritten.

Subsequent findings create new audits.

---

### Bootstrap Policy

Every new engineering session begins by reviewing:

1. `VISION.md` (destination only)
2. `SOVEREIGN_BOOTSTRAP.md`
3. `PROJECT_STATUS.md`
4. `IMPLEMENTATION_MATRIX.md`
5. Latest Audit
6. `IMPLEMENTATION_ROADMAP.md`
7. Active service specification (when one exists)

---

### Non-Negotiable Product Principles

These principles are binding for product design and implementation:

1. Human authority remains final over consequential actions.
2. Local-first operation; owner controls primary data.
3. AI models are replaceable adapters; they do not define system identity.
4. Consequential actions must be justifiable and, where policy requires,
   explicitly approved by a human.
5. Important decisions leave traceable evidence.
6. Runtime primitives with similar names are not product services.
7. Specs and acceptance criteria precede substantial implementation for new
   product services.
8. Simulated or conversational progress is never recorded as repository status.

---

### Authority Boundaries

**May proceed under engineering authority:**

- Repository organization and documentation hygiene
- Runtime maintenance within the closed Runtime Platform v1 boundary
- Service specifications and acceptance tests for the active milestone
- Implementation of the active milestone under an approved specification
- Test infrastructure and validation
- Refactors that preserve public contracts

**Requires owner approval:**

- Changes to constitutional / identity kernel principles
- Changes to human-authority or trust model
- Security philosophy or data-ownership policy changes
- Licensing changes
- Expanding scope beyond the active milestone without an explicit decision
- Any claim that a product service is complete without meeting Definition of Done

---

## Expected Result

Any engineer or AI assistant should be able to resume work with minimal
rediscovery, without treating chat history or design archives as implementation
status.
