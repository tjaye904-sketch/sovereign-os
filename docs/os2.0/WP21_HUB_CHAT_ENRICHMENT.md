# WP-21 — Mission Control chat + live plan enrichment

## Baseline
- Priority 1–5 complete: honest enrichment statuses, headless desktop smoke, roadmap status, 39/39 CTest.
- Branch tip example: `50681e1` on `merge/combined-repos`.

## Goals
1. Mission Control chat must never print shell "command not found".
2. Chat routes to `sovereign-cli chat` (and optional `chat-tools`) with env from `~/.config/sovereign/env`.
3. With Ollama up, `sovereign-cli plan` can reach `enrichment=model_enriched` (or clear `model_unavailable` with reason).
4. Evidence scripts under `scripts/verify-wp21-*.sh`.

## Non-goals
- Bare-metal ISO / host disk wipe.
- Replacing Python CU with C++ purpose runtime.

## Acceptance
- [ ] `curl` or hub UI chat returns model text, not "command not found"
- [ ] `bash scripts/verify-wp21-hub-chat.sh` PASS
- [ ] `bash scripts/verify-wp21-plan-enrichment.sh` PASS when Ollama healthy
- [ ] CTest still 39/39 (or current full suite green)
- [ ] git commit on active branch
