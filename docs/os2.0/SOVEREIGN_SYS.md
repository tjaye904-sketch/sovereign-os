# sovereign-sys v0 — OS operator (headed)

## Design
- All GUI actions open **on the real desktop** (`SOVEREIGN_SYS_HEADLESS=0`).
- Policy allowlists for apps, paths, and user systemd units.
- Audit JSONL under `$SOVEREIGN_DATA_ROOT/sys/audit/`.
- Server: `http://127.0.0.1:8767`

## Visibility rule
When Sovereign is told to use an app/program, it uses `xdg-open` / exec with the session `DISPLAY`/`WAYLAND_DISPLAY` so windows appear for live monitoring. No hidden headless GUI automation in v0.

## Tools
| Action | Risk | Notes |
|--------|------|--------|
| sys_status, sys_processes, sys_services, sys_logs | low | read-only |
| files_list, files_read | low | read roots only |
| app_open, app_focus | medium | **visible** on screen |
| files_write | high | write roots only |
| service_* | high | allowlisted user units |
| run_command | critical | requires --approved |

## Not yet full NixOS control plane
v0 is the Pop/host operator. NixOS rebuild/apply comes after this surface is stable.
