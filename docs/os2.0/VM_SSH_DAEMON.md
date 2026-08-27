# Full VM: SSH + data root + daemon

## Rebuild

```bash
cd ~/src/sovereign-os/sovereign-work
git add nix/modules/sovereign-system.nix scripts/run-sovereign-vm-ssh.sh
nix eval "path:$PWD#packages.x86_64-linux.sovereign-vm.drvPath"
bash scripts/build-sovereign-vm.sh full
bash scripts/run-sovereign-vm-ssh.sh full
```

## Host terminal (copy/paste)

```bash
ssh -p 2222 sovereign@127.0.0.1   # password: sovereign
```

## Inside guest

```bash
echo $SOVEREIGN_DATA_ROOT   # should be /var/lib/sovereign
ls -la /var/lib/sovereign
systemctl status sovereign-daemon --no-pager
sovereign-cli status
```

Daemon starts on boot when the package is present. Ollama defaults to host gateway `http://10.0.2.2:11434`.
