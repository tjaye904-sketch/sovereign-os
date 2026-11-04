# EVIDENCE WP-05 — Inventory + allow/deny

## Spec
Startup/on-demand scan of installed apps (.desktop, PATH, optional Flatpak)
persists to data_root/inventory and app_policy.json with allow/deny toggles
enforced on sys app_open.

## Tests
```bash
sovereign-inventory scan
sovereign-inventory list | head
sovereign-inventory allow gnome-system-monitor
sovereign-inventory deny some-app
bash scripts/verify-wp05-inventory.sh
```

## Results
| Run | Date | Pass/Fail | Notes |
|-----|------|-----------|-------|
| 1 | | | |

## Commit
`WP-05: inventory allow/deny evidence`
