# Sovereign Implementation Matrix

**Purpose:** Traceability between architecture and implementation.  
**v1 product surface (owner 2026-07-29):** CLI-only, Hermes separate, journal sync, hardware-tuned Argon2.

| Component | Architecture | Code | Tests | Status | Notes |
|-----------|--------------|------|-------|--------|-------|
| Runtime Lifecycle | ✓ | ✓ | ✓ | Complete | Runtime v1 |
| Event Infrastructure | ✓ | ✓ | ✓ | Complete | Runtime v1 |
| Diagnostics | ✓ | ✓ | ✓ | Complete | Runtime v1 |
| Scheduling | ✓ | ✓ | ✓ | Complete | Runtime v1 |
| Capability Invocation | ✓ | ✓ | ✓ | Complete | Runtime policy service |
| Provider Selection | ✓ | ✓ | ✓ | Complete | Runtime provider selection |
| Runtime Identity | ✓ | ✓ | ✓ | Complete | Infrastructure only |
| Identity Service | ✓ | ✓ | ✓ | Validated | Argon2id hardware-tuned |
| Governance Service | ✓ | ✓ | ✓ | Implemented | Request/Approve/Reject/ListPending |
| Policy Engine | ✓ | ✓ | ✓ | Implemented | Role+action v1 |
| Memory Engine | ✓ | ✓ | ✓ | Implemented | Append/tag file store |
| Workspace Service | ✓ | ✓ | ✓ | Implemented | Create/list/archive |
| Archive Service | ✓ | ✓ | ✓ | Implemented | Artifact registry |
| Model Router | ✓ | ✓ | ✓ | Implemented | Ollama+tools; host-live |
| Cognitive (C/R/D/R) | ✓ | ✓ | ✓ | Implemented | Tight Governance coupling |
| Action Executor | ✓ | ✓ | ✓ | Implemented | Approved-only execute |
| Synchronization | ✓ | ✓ | ✓ | Implemented | Peer folder bundles + journal |
| Product Host | ✓ | ✓ | ✓ | Implemented | SovereignHost |
| Semantic Memory | ✓ | ✓ | ✓ | Implemented | Local embedder + store |
| Domain Agents | ✓ | ✓ | ✓ | Implemented | Per-domain prompts/tools |
| Peer Bundle Sync | ✓ | ✓ | ✓ | Implemented | Export/import durable bundles |
| sovereign-ctl | ✓ | ✓ | ✓ | Implemented | Ops: bootstrap/start/stop/smoke |
| Evidence pack | ✓ | ✓ | ✓ | Implemented | Approve→archive+memory+semantic |
| sovereign-daemon | ✓ | ✓ | ✓ | Implemented | Long-running agent loop |
| Agent Scheduler | ✓ | ✓ | ✓ | Implemented | Recurring brief jobs + agent-tick |
| Memory Search | ✓ | ✓ | ✓ | Implemented | Keyword search |
| Domain workspaces | ✓ | ✓ | ✓ | Implemented | workspace-init-domains |
| Goals Service | ✓ | ✓ | ✓ | Implemented | Long-lived priorities register 2026-07-30 |
| Briefing Service | ✓ | ✓ | ✓ | Implemented | System brief aggregator 2026-07-30 |
| Audit Log | ✓ | ✓ | ✓ | Implemented | Append-only product audit 2026-07-30 |
| CLI (sovereign-cli) | ✓ | ✓ | ✓ | Implemented | v1 surface; smoke-v1-complete |
| API Gateway | ✓ | — | — | Deferred | Owner: CLI-only v1 |
| Desktop Application | ✓ | — | — | Deferred | Owner: CLI-only v1 |
| Hermes bridge | ✓ | — | — | Deferred | Owner: separate |
| Multi-device peer sync | ✓ | ✓ | ✓ | Implemented | peer-export/import folder bundles 2026-07-30 |

## v1 completion criterion

`scripts/smoke-v1-complete.sh` passes offline (no Ollama required for core path).  
Model commands require host Ollama + `openclaw` (or `SOVEREIGN_OLLAMA_MODEL`).

## Maintenance

Update this matrix when implementation status changes.
