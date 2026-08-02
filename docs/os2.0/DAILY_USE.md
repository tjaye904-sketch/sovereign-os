# Sovereign OS — daily use (step 4)

## On Pop!_OS host

```bash
cd ~/src/sovereign-os/sovereign-work
export SOVEREIGN_DATA_ROOT="$HOME/.local/share/sovereign"
./build/apps/cli/sovereign-cli status
./build/apps/cli/sovereign-cli login <name> <secret>
./build/apps/cli/sovereign-cli brief
./build/apps/cli/sovereign-cli plan "Ship battery project review"
./build/apps/cli/sovereign-cli memory-rank
./build/apps/cli/sovereign-cli inspect
./build/apps/cli/sovereign-cli audit-verify
```

Daemon (optional):

```bash
./build/apps/daemon/sovereign-daemon 60
```

## Live ISO / QEMU

Login `sovereign` / `sovereign`, then same CLI. Data under `/var/lib/sovereign` on v2 images.

## Rebuild ISO (step 1)

```bash
cd ~/src/sovereign-os/sovereign-work
# ensure nix module current
bash scripts/build-sovereign-nix-iso.sh
bash scripts/test-sovereign-iso-boot.sh dist/Sovereign-OS-nix-amd64.iso
```

First ISO build already proved boot; rebuild picks up orchestrator + policy/audit packages after `git add` + commit so the flake sees sources.
