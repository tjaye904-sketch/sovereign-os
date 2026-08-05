# F.4 — Worktree archive / backup

```bash
cd ~/src/sovereign-os/sovereign-work
bash scripts/archive-worktree.sh
# → ~/Downloads/Sovereign-worktree-<hash>-<utc>.zip
# → matching .txt manifest

# Include 1.4G ISO (optional):
bash scripts/archive-worktree.sh --with-iso

# Source only (no git bundle):
bash scripts/archive-worktree.sh --no-git
```

Default zip contains:

- `git archive` of **HEAD** (all tracked source)
- `sovereign.bundle` (full git history, optional)

Does **not** include `build/` by default. ISO only with `--with-iso`.

Restore:

```bash
mkdir -p ~/src/sovereign-restore && cd ~/src/sovereign-restore
unzip ~/Downloads/Sovereign-worktree-*.zip
# if bundle present:
# git clone sovereign.bundle sovereign-work
```
