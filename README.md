# Sovereign (work sandbox)

Local-first, human-governed product services on top of the Runtime Platform.

**This tree is the active engineering sandbox.** It is not a promotion of the
original archive.

## Quick host build (Ollama machine)

```bash
bash scripts/build-sovereign-cli.sh
export OLLAMA_HOST=http://127.0.0.1:11434
export SOVEREIGN_OLLAMA_MODEL=openclaw
./build/sovereign-cli status
./build/sovereign-cli models
./build/sovereign-cli chat "Reply with PONG"
./build/sovereign-cli code-assist --json -- "What is RAII?"
./build/sovereign-cli project-summary .
./build/sovereign-cli operator-create helper pass
./build/sovereign-cli principals
```

See `docs/project/HOST_BUILD.md` and `docs/project/OLLAMA_INTEGRATION.md`.

## Layout

- `core/identity` — Argon2id identity
- `core/governance` / `core/policy` / `core/cognitive` / `core/executor`
- `core/model_router` — local Ollama + tools
- `core/product` — SovereignHost composition
- `apps/cli` — sovereign-cli

## Governance rule

Consequential actions: `recommend` → `approve` → `execute`.
