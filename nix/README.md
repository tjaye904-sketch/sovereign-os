# Nix layout

**Canonical flake is at the repository root** (`../flake.nix`).

```bash
cd ~/src/sovereign-os/sovereign-work
nix eval "path:$PWD#packages.x86_64-linux.sovereign-vm.drvPath"
nix build "path:$PWD#sovereign-vm-minimal" -L
```

This directory still holds modules and packages used by the root flake.
