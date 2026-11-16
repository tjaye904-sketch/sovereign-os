# Host maturity — closing the remaining ~25–30%

## Target (this laptop, Pop!_OS)

Governed cognitive host with Mission Control, chat, computer-use, inventory,
daemon autostart — **not** bare-metal Nix install.

## Package actions

| Item | Mechanism |
|------|-----------|
| Autostart reliability | `sovereign-ensure-ready` + user systemd + linger |
| Mission Control chat UI | `~/.local/share/sovereign/hub-ui/index.html` → `:8768` |
| Chat bridge | WP-21 `sovereign-hub-chat` + optional API |
| Env for systemd | `~/.config/sovereign/env` without `export` |
| Plan enrichment | Longer timeouts; rebuild CLI if sources patched |
| Purpose packs | Shared under `~/.local/share/sovereign/purposes` |
| Merge | `scripts/merge-combined-to-main.sh` when tests green |
| Verify | `scripts/verify-host-maturity.sh` |

## One command

```bash
bash scripts/host-finish-all.sh
bash scripts/verify-host-maturity.sh
```

## Still manual / optional

- Point any *old* hub on :8766 at the new UI or open `sovereign-mission-control`
- Ollama model pull if missing
- `bash scripts/merge-combined-to-main.sh` when you want main updated
