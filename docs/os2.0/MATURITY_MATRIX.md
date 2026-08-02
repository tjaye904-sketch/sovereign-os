# Sovereign maturity matrix (honest)

Target: OS 2.0 complete operating system. **Not 100% of all 26 phases.**

| Phase | Name | Maturity | Evidence |
|-------|------|----------|----------|
| I | Boot / ISO | **High** | NixOS ISO builds + QEMU PASS |
| II | OS runtime | **High** | ServiceOrchestrator in daemon |
| III | Identity | **Medium** | Owner/operator/service/machine; no MFA/TPM yet |
| IV | Governance/policy | **Medium-High** | Versioned rules, dry-run; no quorum |
| V | Memory | **Medium-High** | Importance, aging, prune; not full episodic graph |
| VI | Cognitive | **Medium** | Heuristic + optional Ollama plan enrichment |
| VII | Model router | **Medium** | Ollama local; no multi-provider ensemble |
| VIII | Agents | **Medium** | Scheduler + daemon ticks |
| IX | Workspaces | **Medium** | Domains + workspaces present |
| X | Tools | **Low-Medium** | Domain tools; not full registry |
| XI | Observability | **High** | inspect transparency report |
| XII | Audit | **Medium-High** | Append-only + hash chain |
| XIII–XVI | Security/net/sync/storage | **Partial** | Peer sync, argon2; not TPM/VPN mesh |
| XVII–XXII | UX/install/pkg/HW | **Low-Medium** | CLI; live ISO; install module sketch |
| XXIII–XXVI | AI platform/reliability/docs/evidence | **Partial** | Local AI + tests + docs growing |

**Approved v1 product scope:** marked complete in `inspect coverage`.

**OS 2.0 to 100%:** remaining work is multi-milestone (installer disk write, Secure Boot keys, MFA, full cognitive loops, package channel, desktop deferred by owner).

This revision advances: LLM-enriched plans, installed-system Nix module, maturity tracking.
