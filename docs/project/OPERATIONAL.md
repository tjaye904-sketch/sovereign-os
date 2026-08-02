# Sovereign OS — Operational Guide

## Prerequisites

- C++23 toolchain, libargon2
- Optional: Ollama with `openclaw` (chat) and `nomic-embed-text` (embeddings)

## First run

```bash
bash scripts/build-sovereign-cli.sh
bash scripts/sovereign-ctl.sh bootstrap owner 'your-secret'
bash scripts/sovereign-ctl.sh start 60
bash scripts/sovereign-ctl.sh status
bash scripts/sovereign-ctl.sh brief
```

## Daily operation

| Need | Command |
|------|---------|
| How are things? | `sovereign-ctl brief` or `sovereign-cli brief` |
| Pending decisions | `sovereign-cli pending-decisions` |
| Approve | `sovereign-cli approve <id> <reason>` |
| Override (Owner) | `sovereign-cli override <id> <reason>` |
| Domain agent | `sovereign-cli domain-chat development "..."` |
| Semantic recall | `sovereign-cli semantic-query "..."` |
| Goals | `sovereign-cli goal-add 80 "..."` |

## Authority model

- Consequential actions: Governance Pending → Approve/Reject/Override
- Domain tools: allowlist only; no silent elevation
- Secrets: Argon2id; never in audit payloads

## Data

Default: `$HOME/.local/share/sovereign`  
Override: `SOVEREIGN_DATA_ROOT`

## Smoke

```bash
bash scripts/sovereign-ctl.sh smoke
```
