# L5 — Mission Control capability panel

## Goal
Hub UI lists capability ids with enable/disable toggles; status reflects registry.

## Design
- Hub API (port 8766):
  - `GET /api/capabilities` → list from `sovereign-cli capability-list`
  - `POST /api/capabilities/{id}` body `{"enabled": true|false}` → enable/disable
  - Existing `GET /api/status` may include `capabilities_summary`
- UI: table id | enabled | description | toggle
- Fail closed: CLI errors → show error, do not invent state
- Same `SOVEREIGN_DATA_ROOT` + `SOVEREIGN_CLI` as daemon/CU

## Acceptance
1. CTest still 100% (hub is Python/host; no C++ break required)
2. `/api/capabilities` returns seeded ids including `host.computer_use`
3. Toggle disable → `capability-gate-check browser_open` → allowed=0
4. Toggle enable → allowed=1
5. Documented in HANDOFF / LEAP_LOG

## Branch
`leap/L5-mission-control-caps`
