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
| 00 | Baseline freeze | done |
| 01 | Session permanence (reboot-proof) | done |
| 02 | Truthful status | done |
| 03 | Chat → tool router | done |
| 04 | Observe after act | done |
| 05 | Curiosity engine | done |
| 06 | Inventory enforce app_open | done |
| 07 | Full cognitive loop integration | done |
| 08 | Unified allowlist (inventory = policy) | done |
| 09 | Purpose library | done |
| 10 | Desktop operator MVP | done |
| 11 | Voice on same router | done |
| 12 | Hardening & daily UX | done |
| 13 | OS 2.0 optional (ISO/VM) | done |

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

## Next-Horizon Roadmap (`L28` – `L31`)

| Leap | Branch | Scope | Status |
| --- | --- | --- | --- |
| L28 | `leap/L28-systemd-caps-service-and-clean-build` | Zero-warning build + persistent `systemd --user` `sovereign-caps-ui.service` + docs/handoff update | done |
| L29 | `leap/L29-autonomous-multi-turn-tool-loop` | Autonomous multi-turn tool execution & synthesis loop in `sovereign-cli chat-tools` | ready |
| L30 | `leap/L30-live-a11y-window-and-piper-audio` | Live Linux window/A11y dispatch & Piper TTS audio playback bindings | queued_after_L29 |
| L31 | `leap/L31-multi-node-enrollment-and-usb-release` | Two-node peer enrollment drill & physical USB flash / bare-metal secondary-machine boot | final_step |
