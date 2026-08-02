# Owner Decisions — 2026-07-30

| # | Topic | Decision | Implementation |
|---|--------|----------|----------------|
| 1 | Continuous runner | **B Daemon** | `sovereign-daemon [tick_seconds]` |
| 2 | Semantic memory | **B Local embeddings** | Hashing-trick 64-d vectors; `semantic-add` / `semantic-query` |
| 3 | Domain agents | **B Per-domain tools/prompts** | `core/domain` + `domain-show` / `domain-chat` |
| 4 | Repo home | **A Sandbox only** | No promote |
