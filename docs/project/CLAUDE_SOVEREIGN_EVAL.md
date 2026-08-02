# Evaluation: Claude Sovereign zip (2026-07-30)

## What Claude built well

| Area | Assessment |
|------|------------|
| **Runtime platform** | Substantial event bus, DLQ, capability invocation — aligns with original runtime substrate |
| **GovernanceEngine** | Strong product design: per-action rules, role ranks, auto-approve, Owner-only Override, decision accountability |
| **IdentityManager** | Clear lifecycle (Provisional/Active/Revoked), optional EventBus publish, store port pattern |
| **Fail-safe roles** | Unknown metadata → Guest (never silent elevation) |

## What not to adopt wholesale

| Area | Reason |
|------|--------|
| Parallel Identity stack | We already have Argon2id IdentityService + file store + sessions |
| In-memory-only stores | Conflicts with long-lived OS durability |
| Trace/capability name chains | Previously classified as bloat / scope trap |
| Replacing SovereignHost/CLI | Our product surface is the cognitive OS path |

## Implemented from Claude into sovereign-work

1. `GovernanceRole` ranks (Guest < Operator < Administrator < Owner)
2. `GovernanceRule` per action (auto-approve + min authority)
3. File-backed `rules.dat` + durable decisions with `is_override`
4. `Override()` Owner-only emergency path
5. CLI: `override`, `rule-set`

## Kept from our line

Identity Argon2, Semantic/Ollama embed, Domains, Goals, Briefing, Daemon, CLI-only surface, journal sync.
