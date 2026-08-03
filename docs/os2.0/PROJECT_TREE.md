# Sovereign project tree — AI / model context refresher

**Repo root (host):** `~/src/sovereign-os/sovereign-work`  
**Data root (runtime):** `$SOVEREIGN_DATA_ROOT` or `~/.local/share/sovereign`  
**Git user:** tjaye904-sketch  

## What Sovereign is

Governance-first local cognitive OS: identity → policy → memory → models (Ollama) → agents → audit.  
**v1 product scope:** complete (see `sovereign-cli inspect`).  
**OS 2.0:** NixOS live ISO boots; full disk OS install and MFA still incomplete.

## Layout (logical)

```
sovereign-work/
├── apps/
│   ├── cli/main.cpp          # sovereign-cli (all user commands)
│   └── daemon/main.cpp       # ServiceOrchestrator + agent tick
├── core/
│   ├── identity/             # Owner/operator/service/machine, sessions, Argon2
│   ├── governance/           # Pending decisions, approve/reject/override
│   ├── policy/               # Versioned rules, dry-run
│   ├── memory/               # Records, importance, aging, prune
│   ├── cognitive/            # Plan, recommend, reflect (+ optional Ollama)
│   ├── model_router/         # Ollama-only chat
│   ├── audit/                # audit.dat + audit.chain
│   ├── os_runtime/           # ServiceOrchestrator
│   ├── product/              # SovereignHost composition root
│   ├── diagnostics/          # inspect transparency report
│   └── ... (workspace, agent, goals, semantic, domain, executor, sync, runtime)
├── nix/
│   ├── flake.nix             # ISO + packages
│   └── modules/
│       ├── sovereign-iso.nix # Live image
│       └── sovereign-system.nix  # Installed system sketch
├── scripts/
│   ├── build-sovereign-nix-iso.sh
│   ├── test-sovereign-iso-boot.sh
│   └── iso/                  # first-boot helpers
├── docs/os2.0/               # Phases, maturity matrix, this tree
├── testing/                  # *_smoke_test.cpp + CTest
└── CMakeLists.txt
```

## Critical CLI surface

```
bootstrap / login / logout / session-revoke
service-create / machine-create / principal-suspend / principal-revoke
status / brief / inspect / audit-verify / audit-list
policy-check / policy-dry-run / policy-version / policy-rule-set
memory-add / memory-rank / memory-prune
plan / loop [--auto-approve] / recommend / approve / execute
```

## Proven gates (host)

- CMake Release + 33 CTest PASS  
- E2E temp data_root scenario PASS (historical)  
- ISO build + QEMU boot PASS  
- loop pause + loop --auto-approve PASS  

## Owner constraints

- All product work stays out of original ChatGPT zip; this is the sovereign-work tree  
- Local Ollama; deferred: http_api, desktop, hermes_bridge  
- No silent disk install; human names device  

## How models should use this file

On context load: read this + `MATURITY_MATRIX.md` + `DAILY_USE.md`.  
Do not re-bootstrap design from zero; extend existing modules and keep governance gates.
