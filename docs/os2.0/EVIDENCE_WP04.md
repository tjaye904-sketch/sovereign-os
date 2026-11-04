# EVIDENCE WP-04 — Purpose packs

## Spec
Reusable purpose YAML packs under data_root/purposes and repo purposes/,
listable and runnable via CU purpose_list / purpose_run.

## Packs
- research.web_collect
- research.wikipedia_brief
- ops.desktop_probe
- ops.health_probe
- ops.system_monitor (notify + window list; sys open via sovereign-tools)

## Tests
```bash
bash scripts/install-purposes.sh   # if needed
sovereign-cu purpose_list
sovereign-cu purpose_run --args '{"purpose_id":"ops.health_probe"}'
bash scripts/verify-wp04-purposes.sh
```

## Results
| Run | Date | Pass/Fail | Notes |
|-----|------|-----------|-------|
| 1 | | | |

## Commit
`WP-04: purpose pack library`
