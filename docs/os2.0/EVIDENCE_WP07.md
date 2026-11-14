# EVIDENCE WP-07 — Inventory CLI format ↔ sys enforcement

## Spec
`sovereign-inventory allow|deny` writes `app_policy.json` with
`{"apps": {"id": {"allow": bool, "bin", "name"}}}`.

`sovereign-sys` policy accepts that shape **and** flat `"allow"|"deny"` strings.
Lookups match **id**, **bin**, and **name**.

## Tests
```bash
sovereign-inventory allow gnome-system-monitor
sovereign-inventory deny gucharmap
sovereign-sys app_open --args '{"target":"gucharmap"}'   # deny
sovereign-sys app_open --args '{"target":"gnome-system-monitor"}'  # open
bash scripts/verify-wp07-inventory-format.sh
```

## Results
| Run | Date | Pass/Fail | Notes |
|-----|------|-----------|-------|
| 1 | | | |

## Commit
`WP-07: inventory CLI format aligned with sys policy`
