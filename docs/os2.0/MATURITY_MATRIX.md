# Sovereign maturity matrix (honest)

**Updated:** 2026-08-05  
**HEAD reference:** post D.2 (`dce785d` + D.1 `758a09e` + C.3 `7fd2ebe`)  
Target: OS 2.0 complete operating system. **Not 100% of all 26 phases.**

| Phase | Name | Maturity | Evidence |
|-------|------|----------|----------|
| I | Boot / ISO | **High** | NixOS ISO 1.4G; QEMU boot **PASS** (D.2) |
| II | OS runtime | **High** | ServiceOrchestrator + smoke test (23 CTest) |
| III | Identity | **Medium–High** | Owner/operator/service/machine; TOTP + recovery codes (C.1–C.3); step-up on execute when enrolled |
| IV | Governance/policy | **Medium–High** | Versioned rules, dry-run; no quorum |
| V | Memory | **Medium–High** | Importance, aging, rank, prune |
| VI | Cognitive | **Medium–High** | Plan + enrichment status; `loop` plan→approve→execute→reflect |
| VII | Model router | **Medium** | Ollama local; timeouts; no multi-provider ensemble |
| VIII | Agents | **Medium** | Scheduler (warning fixed); daemon ticks |
| IX | Workspaces | **Medium** | Domains + workspaces |
| X | Tools | **Low–Medium** | Domain tools; not full registry |
| XI | Observability | **High** | `inspect` transparency + **per-command wall timing** (D.1) |
| XII | Audit | **Medium–High** | Append-only + hash chain; sensitive CLI paths audited (A.3) |
| XIII–XVI | Security/net/sync/storage | **Partial** | Peer sync, Argon2; MFA recovery; not TPM/VPN mesh |
| XVII–XXII | UX/install/pkg/HW | **Medium** | CLI; live ISO; installer **gates** (B.1); VM path (B.2); first-boot (B.3); host disk still gated |
| XXIII–XXVI | AI platform/reliability/docs/evidence | **Medium** | Local AI + 23 CTest + docs; clean rebuild verified |

## Milestone program (this branch)

| ID | Topic | Status |
|----|--------|--------|
| A.1–A.5 | Revoke, plan enrichment, audit paths, runtime quarantine, PROJECT_TREE | **Done** |
| B.1–B.3 | Installer dry-run/VM gates, QEMU VM flake, first-boot | **Done** |
| C.1–C.3 | MFA scaffold, TOTP, recovery + step-up | **Done** |
| Post-C.3 | Agent Load fix, orchestrator smoke | **Done** |
| D.1 | Per-command wall timing | **Done** |
| D.2 | ISO rebuild + boot PASS | **Done** |
| E.1–E.3 | OS 2.0 VM disk / boot-install / installed path | **Done** |
| E.4 | Host install gates (no wipe) | **Done (gated)** |
| F.1 | Secure Boot / measured boot docs + probes | **Done (docs)** |
| F.2 | Optional require-MFA login | **Done** |
| F.3 | Package channel / flake pins | **Done (docs+verify)** |
| F.4 | Worktree archive script | **Done** |

## Scope scores (honest)

| Scope | Approx. |
|-------|---------|
| Approved **v1 product** (CLI host) | **~100%** (`inspect coverage`) |
| **OS 2.0** (bootable installable OS) | **~55–60%** (live ISO + VM install path; host wipe still gated) |
| Full life-OS vision | **~35–40%** |

## Explicit remaining (not claimed done)

- Host bare-metal `nixos-install` with confirmed disk wipe  
- Secure Boot key management  
- Mandatory MFA on all logins (optional enroll works)  
- Quorum governance  
- Desktop / http_api / hermes_bridge (deferred by owner)  
- Experimental `core/runtime` event bus (quarantined, optional ON)
