# Sovereign OS — Completion Pathway (JARVIS-aligned)

**Rule:** One work package at a time. Finish G1–G7 before starting the next WP.

**North star:** Conversation is the UI; the host desktop is the workspace; identity/policy/audit are authority; models reason; sys + computer-use (+ desktop drive) act; curiosity + observe make the system learn.

**Primary target:** Pop!_OS host. Nix ISO/VM = WP-13 only after WP-00–12.

## Gates (every WP)

| Gate | Requirement |
|------|-------------|
| G1 Spec | Goal, interfaces, non-goals documented |
| G2 Implement | Code/scripts installed |
| G3 Build | cmake/package install OK if applicable |
| G4 Test | Package smokes pass |
| G5 Evidence | `docs/os2.0/EVIDENCE_WPXX.md` |
| G6 Surface | CLI and/or Mission Control path works |
| G7 Commit | `WP-XX: …` on main (or host-config note) |

## Work package order (strict)

| WP | Name | Status |
|----|------|--------|
| 00 | Baseline freeze | in_progress |
| 01 | Session permanence (reboot-proof) | not_started |
| 02 | Truthful status | not_started |
| 03 | Chat → tool router | not_started |
| 04 | Observe after act | not_started |
| 05 | Curiosity engine | not_started |
| 06 | Reflect → evolve | not_started |
| 07 | Full cognitive loop integration | not_started |
| 08 | Unified allowlist (inventory = policy) | not_started |
| 09 | Purpose library | not_started |
| 10 | Desktop operator MVP | not_started |
| 11 | Voice on same router | not_started |
| 12 | Hardening & daily UX | not_started |
| 13 | OS 2.0 optional (ISO/VM) | blocked_until_12 |

Update **Status** only when closing a WP: `not_started` → `in_progress` → `done`.

Only **one** `in_progress` at a time.

## Cross-cutting (every WP)

1. Human authority on consequential acts  
2. Visible/headed actuation when UI is claimed  
3. Audit act + observe  
4. Local Ollama unless explicitly changed  
5. No host disk wipe  
6. Pop host first  
7. Curiosity probes ⊆ policy  
8. JARVIS test: saying the goal out loud should move the lab  

## Tracking

See also:

- `docs/os2.0/PROJECT_TREE.md` — tree + WP map  
- `docs/os2.0/ALIGNMENT_JARVIS.md` — design vs build  
- `docs/os2.0/EVIDENCE_WPXX.md` — per-WP evidence  
