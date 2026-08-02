# Owner Decision — Peer Sync Opened (2026-07-30)

| Topic | Decision |
|-------|----------|
| Peer sync | **Approved** — local-first **folder peer bundles** |
| Transport | Directory bundles (`peer-export` / `peer-import`); journal records channel `peer-folder` |
| Secrets | **Never** in bundles |
| Auto-execute / auto-approve | **Forbidden** on import |
| Network mesh | Not enabled by default (ideal per local-first / threat posture) |

Ideal options retained from prior locks: CLI-only, Hermes separate, hardware Argon2, daemon, local embeddings, domain agents, sandbox.
