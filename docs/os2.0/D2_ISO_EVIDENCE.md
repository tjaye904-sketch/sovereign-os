# D.2 — ISO rebuild + boot evidence

## Commands

```bash
cd ~/src/sovereign-os/sovereign-work
# ensure dist writable
mkdir -p dist && chmod u+w dist || true
bash scripts/build-sovereign-nix-iso.sh
bash scripts/test-sovereign-iso-boot.sh dist/Sovereign-OS-nix-amd64.iso
```

## Pass criteria

- `[nix-iso] DONE: .../Sovereign-OS-nix-amd64.iso` with non-zero size
- `[boot-test] PASS — boot markers found`
- Serial log shows NixOS/Sovereign live markers

## Notes

- First build can take 30–90+ minutes
- Needs Nix flakes + qemu-system-x86_64
- Does not write host disks
