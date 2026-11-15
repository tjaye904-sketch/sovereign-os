# EVIDENCE WP-03 — Chat → tool router

## Spec
Natural language maps to CU/sys tool calls without claiming unrestricted host access.
Unknown intents return `no_tool_match` + hint, not a pretend answer.

## Deliverables
- `scripts/sovereign_tool_router.py` / `sovereign-tools` CLI
- Pattern routes: browser.open, purpose.run, browser.snapshot, desktop.list_windows, app.launch, ping
- Policy still enforced by CU/sys services

## Tests
```bash
sovereign-tools --dry-run "open python docs"
sovereign-tools "ping"
bash scripts/verify-wp03-router.sh
```

## Results
| Run | Date | Pass/Fail | Notes |
|-----|------|-----------|-------|
| 1 | | | |

## Commit
`WP-03: chat-to-tool router (sovereign-tools)`
