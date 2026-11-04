# EVIDENCE WP-01 — Session permanence

## Spec
After graphical login, daemon, hub, sys, and computer-use are reachable without
manual `nohup`, and headed browser_open works.

## Install
- `scripts/sovereign-session-start.sh` → `~/.local/bin/sovereign-session-start`
- `~/.config/autostart/sovereign-session.desktop`
- linger enabled
- CU/sys/hub nohup fallbacks in session-start

## Tests
1. `sovereign-session-start` then `scripts/verify-wp01-session.sh`
2. Reboot, login, wait 15s, re-run verify (twice)

## Results

| Run | Date | Pass/Fail | Notes |
|-----|------|-----------|-------|
| 1 (live) | 2026-11-04 ~02:37 EST | PASS | 8765/8766/8767 OK; daemon active; browser_open PASS |
| 1b (after sys reinstall) | 2026-11-04 ~02:35 EST | PASS | Fixed ModuleNotFoundError sovereign_sys |
| reboot 1 | _pending_ | | After `systemctl reboot` + graphical login + 15s |
| reboot 2 | _pending_ | | Second full reboot |

## Commit
`WP-01: session-start + autostart for reboot-proof operators`# EVIDENCE WP-01 — Session permanence

## Spec
After graphical login, daemon, hub, sys, and computer-use are reachable without manual `nohup`, and headed browser_open works.

## Install
- `scripts/sovereign-session-start.sh` → `~/.local/bin/sovereign-session-start`
- `~/.config/autostart/sovereign-session.desktop`
- linger enabled

## Tests
1. `sovereign-session-start` then `scripts/verify-wp01-session.sh`
2. Reboot, login, wait 15s, re-run verify (twice)

## Results
| Run | Date | Pass/Fail | Notes |
|-----|------|-----------|-------|
| 1 | | | |
| reboot 1 | | | |
| reboot 2 | | | |

## Commit
`WP-01: session-start + autostart for reboot-proof operators`
