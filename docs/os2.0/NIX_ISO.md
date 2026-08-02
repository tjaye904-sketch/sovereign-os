# Sovereign OS — NixOS ISO

## Verified in development sandbox (2026-08-02)

| Check | Result |
|-------|--------|
| Flake evaluates | PASS (`nix flake show`) |
| `sovereign-cli` Nix package builds | PASS |
| CLI runs (`status`) | PASS |
| Full ISO image build | **Run on Pop!_OS host** (needs 8+ GB RAM, ~20 GB disk) |
| QEMU boot test | After ISO exists: `bash scripts/test-sovereign-iso-boot.sh` |

## On Pop!_OS (Dell)

```bash
cd ~/src/sovereign-os/sovereign-work

# Install Nix (once)
curl -L https://nixos.org/nix/install | sh -s -- --daemon
# log out/in, then:
mkdir -p ~/.config/nix
echo 'experimental-features = nix-command flakes' >> ~/.config/nix/nix.conf

# Sync nix/ from this tree if needed, then:
bash scripts/build-sovereign-nix-iso.sh
bash scripts/test-sovereign-iso-boot.sh dist/Sovereign-OS-nix-amd64.iso

# Manual boot:
qemu-system-x86_64 -m 4096 -smp 4 -cdrom dist/Sovereign-OS-nix-amd64.iso -boot d
```

Live login: `sovereign` / `sovereign`
