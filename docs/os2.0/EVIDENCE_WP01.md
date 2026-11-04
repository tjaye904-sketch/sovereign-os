# EVIDENCE WP-01 — Session permanence

## Spec
After graphical login, daemon, hub, sys, and computer-use are reachable without
manual `nohup`, and headed browser_open works.

## Install
- `scripts/sovereign-session-start.sh` → `~/.local/bin/sovereign-session-start`
- `~/.config/autostart/sovereign-session.desktop` (DISABLED after instability)
- linger: use with caution; prefer disable until StartLimit is set
- CU/sys/hub nohup fallbacks in session-start

## Tests
1. `sovereign-session-start` then `scripts/verify-wp01-session.sh`
2. Reboot ×2 — **SUSPENDED** (sys restart storm / recovery risk)

## Results

| Run | Date | Pass/Fail | Notes |
|-----|------|-----------|-------|
| 1 (live) | 2026-11-04 ~02:37 EST | PASS | 8765/8766/8767 OK; daemon active; browser_open PASS |
| 1b (sys reinstall) | 2026-11-04 ~02:35 EST | PASS | Fixed ModuleNotFoundError sovereign_sys |
| reboot 1 | — | SUSPENDED | Not run; safety hold |
| reboot 2 | — | SUSPENDED | Not run; safety hold |

## Follow-ups before re-enabling autostart
- `Restart=on-failure` + `StartLimitBurst` / `StartLimitIntervalSec` on sys & CU units
- Autostart off by default; manual or delayed one-shot only
- Confirm no crash loop: `systemctl --user status sovereign-sys`

## Commit
`WP-01: session-start + live verify PASS; reboot tests suspended`
