# Host recovery (Pop!_OS path)

## Data
- Data root: `$SOVEREIGN_DATA_ROOT` (default `~/.local/share/sovereign`)
- Backup: `bash scripts/archive-worktree.sh` (repo) + copy data root separately
- Peer: `sovereign-cli peer-export /path`

## Services
```bash
systemctl --user restart sovereign-daemon
~/.local/bin/sovereign-ensure-ready
curl -s http://127.0.0.1:8765/health
curl -s http://127.0.0.1:8768/health
```

## Identity
- Login: `sovereign-cli login owner '<secret>'`
- MFA recovery codes if enrolled: `mfa-recovery-generate` (store offline)
- Revocation: `session-revoke` / `principal-suspend` (owner)

## Ollama
```bash
systemctl --user start ollama 2>/dev/null || sudo systemctl start ollama
ollama list
export SOVEREIGN_OLLAMA_MODEL=...   # match a listed tag
```

## Do not
- Wipe host disks / run installers without gates
- Force-push git without reviewing origin divergence
