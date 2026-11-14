# EVIDENCE WP-11 — Daily operator path

## Spec
One script proves the host operator stack end-to-end:
1. systemd units / health endpoints
2. inventory allow for system monitor
3. optional purpose_run (headed browser)
4. tools/sys open system monitor on screen
5. evidence log under data_root/evidence/

## Test
```bash
bash scripts/daily-operator-path.sh
```

## Commit
`WP-11: daily operator path E2E smoke`
