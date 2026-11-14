# EVIDENCE WP-10 — Session stack autostart

## Spec
After graphical login, without manual starts:
- sovereign-daemon active
- hub :8766 status JSON
- computer-use :8765 health
- sys :8767 health
- Mission Control markers not all red

## Units
- sovereign-daemon.service (existing)
- sovereign-sys.service
- sovereign-computer-use.service
- sovereign-hub.service
- linger enabled for user

## Tests
```bash
bash scripts/verify-wp10-autostart.sh
# after reboot optional: same checks
```

## Commit
`WP-10: user systemd units for hub/cu/sys autostart`
