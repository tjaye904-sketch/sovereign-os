# EVIDENCE WP-02 — Truthful status

## Spec
Status must report **capability**, not only TCP listen:
- `computer_use.ready` requires health OK **and** graphical session env (DISPLAY/WAYLAND in shell or systemd user env)
- `sys.ready` requires health OK
- `operators_ready` is true only when daemon + sys.ready + computer_use.ready
- `reasons_not_ready` lists gaps

## Deliverables
- `scripts/sovereign-capability-status.py`
- `~/.local/bin/sovereign-capability-status`
- Optional hub route `GET /api/capability`
- `scripts/verify-wp02-status.sh`

## Tests
```bash
python3 scripts/sovereign-capability-status.py --pretty
bash scripts/verify-wp02-status.sh
```

## Results
| Run | Date | Pass/Fail | Notes |
|-----|------|-----------|-------|
| 1 | | | |

## Commit
`WP-02: capability status (listening vs ready)`
