# EVIDENCE WP-06 — Inventory enforce on app_open

## Spec
`sovereign-sys` `app_open` consults `data_root/inventory/app_policy.json`:
- explicit **deny** → blocked
- explicit **allow** → permitted (risk tier still applies)
- inventory present, app unknown → **deny-by-default**
- no inventory file → static `DEFAULT_ALLOWED_APPS` only

## Tests
```bash
bash scripts/verify-wp06-inventory-enforce.sh
sovereign-inventory allow gnome-system-monitor
sovereign-sys app_open --args '{"target":"gnome-system-monitor"}'
```

## Results
| Run | Date | Pass/Fail | Notes |
|-----|------|-----------|-------|
| 1 | | | |

## Commit
`WP-06: enforce inventory allow/deny on sys app_open`
