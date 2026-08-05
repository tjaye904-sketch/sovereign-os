# OS 2.0 — Package channel & flake pins (F.3)

## Channel model

Sovereign does **not** ship a separate binary APT repo. The **package channel** is the **Nix flake**:

| Artifact | Flake output |
|----------|----------------|
| Live ISO | `.#iso` → `nixosConfigurations.sovereign-iso` |
| Installed system | `nixosConfigurations.sovereign-system` |
| QEMU VM | `.#sovereign-vm` |
| CLI / daemon packages | `.#sovereign-cli` / `.#sovereign-daemon` |

**nixpkgs pin:** `github:NixOS/nixpkgs/nixos-24.11` (see `nix/flake.nix` + `nix/flake.lock`).

## Pinning rules

1. **Always commit `nix/flake.lock`** with any flake or ISO change.  
2. Prefer **branch** `nixos-24.11` over `nixos-unstable` for reproducibility.  
3. Bump channel only intentionally (security or needed packages).  
4. After bump: rebuild ISO or VM and re-run boot smoke when practical.

## Update procedure

```bash
cd ~/src/sovereign-os/sovereign-work/nix

# Show current lock
nix flake metadata .
# or: jq -r '.nodes.nixpkgs...' flake.lock

# Fetch newer nixos-24.11 revision into the lock
nix flake update nixpkgs

# Review
git diff flake.lock

# Evaluate still works (no full ISO required)
nix flake check . 2>/dev/null || nix eval .#packages.x86_64-linux.sovereign-cli --raw

# Commit
git add flake.lock
git commit -m "nix: update nixpkgs pin (nixos-24.11)"
```

## Host C++ path (non-Nix)

Pop!_OS / Debian builds use **system packages** (`libargon2`, cmake, g++). Those are **not** the flake channel; track with:

```bash
apt-cache policy libargon2-dev cmake g++
```

## Verification

```bash
bash scripts/verify-package-channel.sh
```

## Explicit non-goals (F.3)

- No third-party APT repository  
- No automatic unattended `flake update` on user machines  
- No unsigned binary CDN  
