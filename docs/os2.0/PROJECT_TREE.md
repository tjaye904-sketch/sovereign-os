# Sovereign OS — Project Tree & Workflow Map

AI refresher + human map. Reflects **JARVIS-aligned completion pathway** (2026-11).

## Repository layout (canonical product tree)

```text
sovereign-work/
├── apps/
│   ├── cli/                 # sovereign-cli — primary operator CLI
│   └── daemon/              # sovereign-daemon — Phase II orchestrator
├── core/
│   ├── identity/            # principals, sessions, MFA, revocation
│   ├── governance/          # decisions, approve/reject
│   ├── policy/              # rules, version, dry-run
│   ├── memory/              # records, rank, prune
│   ├── semantic/            # embeddings / query
│   ├── cognitive/           # plan, loop; WP-05 curiosity; WP-06 reflect
│   ├── goals/               # goal store
│   ├── workspace/           # workspaces + domains
│   ├── audit/               # append-only audit
│   ├── agent/               # scheduler ticks
│   ├── model_router/        # Ollama client
│   ├── executor/            # governed execute
│   ├── product/             # SovereignHost composition
│   ├── diagnostics/         # transparency / timing
│   ├── os_runtime/          # ServiceOrchestrator
│   └── …
├── nix/                     # ISO/VM flakes (WP-13)
├── scripts/
│   ├── iso/                 # install/boot helpers
│   ├── sovereign-session-start.sh   # WP-01 host presence
│   ├── verify-wp01-session.sh
│   └── …
├── docs/os2.0/
│   ├── COMPLETION_PATHWAY.md        # sequential WP plan
│   ├── PROJECT_TREE.md              # this file
│   ├── ALIGNMENT_JARVIS.md
│   ├── BLUEPRINT_AND_ALIGNMENT.md
│   ├── HOST_AUTOMATION.md
│   ├── MATURITY_MATRIX*.md
│   └── EVIDENCE_WP*.md
├── purposes/                # CU purpose YAML (WP-09)
├── testing/                 # CTest smokes
├── CMakeLists.txt
├── flake.nix                # optional root flake
└── README.md
```

## Host-side operators (not always in git; install to ~/.local)

```text
~/.local/bin/
  sovereign-cli, sovereign-daemon
  sovereign-hub-server, sovereign-hub-open
  sovereign-cu, sovereign-cu-server
  sovereign-sys, sovereign-sys-server
  sovereign-inventory
  sovereign-session-start          # WP-01
~/.config/systemd/user/
  sovereign-daemon.service
  sovereign-hub.service
  sovereign-sys.service
  sovereign-computer-use.service
~/.config/autostart/
  sovereign-session.desktop        # WP-01
~/.local/share/sovereign/          # data_root
```

## Workflow plan → tree ownership

| WP | Primary paths | Outcome |
|----|---------------|---------|
| 00 | docs/os2.0/BASELINE_*.md | Freeze known state |
| 01 | scripts/sovereign-session-start.sh, autostart, systemd | Reboot-proof presence |
| 02 | hub status + CU health capability flags | Truthful online |
| 03 | hub `/api/command` router; optional cli | Chat operates machine |
| 04 | router + audit observe payloads | Act → observe |
| 05 | core/cognitive/curiosity_* ; CLI `curious` | Gap → probe |
| 06 | reflect → memory #learned | Evolve |
| 07 | apps/cli loop phases 1–7 | Full cognitive cycle |
| 08 | sys policy + inventory single source | One allowlist |
| 09 | purposes/*.yaml | Mission packs |
| 10 | desktop operator module | Type/focus native apps |
| 11 | voice → same router | Hands-free intent |
| 12 | docs DAILY_DRIVER, hardening | Cold-boot UX |
| 13 | nix/, scripts/iso/ | Optional portable OS |

## Cognitive loop (target after WP-07)

```text
curious → plan → govern → act → observe → reflect → evolve
```

## JARVIS rule

If a change does not help **intent → visible governed work on the host**, it does not belong in WP-00–12.

## Status

Pathway authority: `docs/os2.0/COMPLETION_PATHWAY.md`  
Last workflow update: 2026-11-04
