# Hermes / Ollama agent alignment for Sovereign OS

## Model env (match Sovereign defaults)

```bash
export OLLAMA_HOST=http://127.0.0.1:11434
export SOVEREIGN_OLLAMA_MODEL=hermes-nemotron-3-supercloud   # or: openclaw, qwen2.5-coder:7b
export SOVEREIGN_EMBED_MODEL=nomic-embed-text
export SOVEREIGN_DATA_ROOT="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}"
```

## System prompt / standing orders (paste into Hermes)

```
You are assisting development of Sovereign OS: a governance-first local
cognitive OS (C++23 CLI/daemon + NixOS live ISO).

Standing rules:
1. Never suggest host disk wipe without DESTROY_DISK + SOVEREIGN_INSTALL_ALLOW_HOST=1.
2. Prefer local Ollama; no cloud API keys required.
3. Product default: SOVEREIGN_BUILD_EVENT_RUNTIME=OFF.
4. Do not re-bootstrap design; extend existing modules under core/, apps/, nix/.
5. Every change should compile with cmake + pass ctest (24 tests).
6. Live ISO QEMU boot is proven; system.build.vm is separate and fragile — prefer
   ISO boot tests unless explicitly fixing nix/modules/sovereign-system.nix.
7. MFA: SOVEREIGN_REQUIRE_MFA=1 optional; recovery codes one-time; no default secrets.
8. Git commits after verified tests; do not force-push main.

Priority order when stuck:
A) Keep product CTest green
B) Live ISO boot PASS
C) Fix nix eval for system.build.vm without breaking ISO
D) Docs in docs/os2.0/

Read first: docs/os2.0/PROJECT_TREE.md, MATURITY_MATRIX.md, PACKAGE_CHANNEL.md
```

## Useful Hermes tasks

```text
1) Diagnose why NixOS systemd ExecStart with ${pkgs.X}/bin/X fails only under
   virtualisation.vmVariant with error: path '/' is not a valid store path
2) Propose a minimal sovereign-system.nix that system.build.vm accepts
3) Review core/identity for SOVEREIGN_REQUIRE_MFA edge cases
4) Draft test plan for scripts/test-sovereign-iso-boot.sh regression
```

## Sovereign CLI against same model

```bash
export OLLAMA_HOST=http://127.0.0.1:11434
export SOVEREIGN_OLLAMA_MODEL=hermes-nemotron-3-supercloud
./build/apps/cli/sovereign-cli models
./build/apps/cli/sovereign-cli chat "Reply with exactly PONG"
./build/apps/cli/sovereign-cli plan "Prioritize remaining OS 2.0 work"
```
