# EVIDENCE WP-08 + WP-09

## WP-08 Mission Control inventory toggles
- UI panel: allow/deny per scanned app
- API: `GET /api/inventory/list`, `POST /api/inventory/set`, `POST /api/inventory/scan`

## WP-09 Tools / chat respect inventory
- `sovereign-tools` pre-checks inventory before `sys app_open`
- Routes: allow/deny/list inventory via natural language

## Tests
```bash
sovereign-tools --json "open gucharmap"          # deny if denied
sovereign-tools "open system monitor"            # open if allowed
bash scripts/verify-wp08-09.sh
# Mission Control: Inventory card toggles
```

## Commit
`WP-08/09: hub inventory toggles + tools inventory precheck`
