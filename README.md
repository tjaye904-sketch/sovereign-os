# Sovereign OS

Sovereign OS is an AI-native operating environment for a trusted local operator.
It is designed to let an intelligent agent act with the initiative and context of
an admin-level operator working at a desktop, while remaining bound by identity,
policy, review, and approval controls.

This is not a chatbot wrapper. It is a local-first governance-controlled digital
workspace that combines memory, planning, execution, and human oversight into one
runtime.

## Product thesis

The intended experience is a Jarvis-like operating model:

- the AI observes the system and understands state
- the AI reasons across memory, goals, and workflows
- the AI executes tasks using local tools and system services
- the AI requests approval before consequential actions
- the human remains the final authority over trust and risk

In practical terms, Sovereign OS is an operating system for a digital operator:
capable, persistent, auditable, and safe under human governance.

## Core principles

- Local-first operation and local ownership of data
- Governance before action
- Human authority over high-impact decisions
- Safe, sandboxed mutation for autonomous improvement
- Evidence-backed reasoning and decision trails
- Model independence with clear service boundaries

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



## Purpose stacks (C++ vs computer-use)

Sovereign has two complementary purpose/automation paths:

- **C++ `core/product/purpose_runtime` + `sovereign-cli purpose-list|purpose-run`** — in-process pack loading, allowlists, and governed evaluation inside the product host. Prefer this for policy-shaped packs and CLI/automation that must stay inside the C++ host boundary.
- **Python computer-use (`sovereign-cu`, port 8765)** — headed browser/desktop tools, Playwright, Mission Control integration, and live on-screen automation on Pop!_OS. Prefer this when the operator must *see* windows and pages move.

Do not assume the two share one allowlist file; keep CU host policy and C++ purpose packs aligned intentionally. Desktop operator (`xdg-open` / snapshots) is a third, thinner actuation path under governance — not a full window driver.

## System layout

- `core/identity` — identity, principals, and session trust
- `core/governance` / `core/policy` / `core/cognitive` / `core/executor` — governance and operator runtime
- `core/model_router` — model selection and local tool routing
- `core/product` — host composition and service orchestration
- `apps/cli` — sovereign CLI surface
- `tools/sovereign_mesh` — isolated, governed task execution model

## Governance rule

Consequential actions follow a clear control flow:

recommend → approve → execute

This keeps the system operationally capable without permitting uncontrolled autonomy.

<!-- merge note 2026-11-18: also integrated origin/main orchestration/cognitive engines -->
