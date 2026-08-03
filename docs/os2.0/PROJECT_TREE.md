# Sovereign project tree — AI / model context refresher

**Updated:** 2026-08-03 (Milestone A.1–A.4 complete)  
**Repo root (host):** `~/src/sovereign-os/sovereign-work`  
**Data root:** `$SOVEREIGN_DATA_ROOT` or `~/.local/share/sovereign`  
**Git user:** tjaye904-sketch  

---

## What Sovereign is

Governance-first **local cognitive OS substrate**: identity → policy → memory → models (Ollama) → agents → audit.  
Bootable **NixOS live ISO** exists; full disk install + MFA remain incomplete.

| Scope | Status |
|-------|--------|
| Approved **v1 product** (CLI host) | ~95–100% |
| **OS 2.0** (bootable installable OS) | ~40% |
| Full life-OS vision | ~30–40% |

---

## Layout

```
sovereign-work/
├── apps/
│   ├── cli/main.cpp           # sovereign-cli (primary surface)
│   ├── daemon/main.cpp        # ServiceOrchestrator + agent/audit ticks
│   └── installer/             # sketch only — no silent disk writes
├── core/
│   ├── identity/              # owner/operator/service/machine, sessions, Argon2
│   ├── governance/            # pending decisions, approve/reject/override
│   ├── policy/                # versioned rules, dry-run
│   ├── memory/                # importance, aging, rank, prune
│   ├── cognitive/             # plan + enrichment_status, recommend, loop, reflect
│   ├── model_router/          # Ollama-only; curl --max-time 25
│   ├── audit/                 # append-only + hash chain
│   ├── os_runtime/            # ServiceOrchestrator (product path)
│   ├── product/               # SovereignHost composition root
│   ├── diagnostics/           # inspect transparency report
│   ├── agent/ goals/ briefing/ semantic/ domain/ workspace/
│   ├── archive/ sync/ executor/
│   └── runtime/               # EXPERIMENTAL event bus — QUARANTINED by default
│       └── _quarantine_trace_bloat/
├── nix/                       # flake ISO + sovereign-system module
├── scripts/
│   ├── build-sovereign-nix-iso.sh
│   ├── test-sovereign-iso-boot.sh
│   ├── verify-revocation-cli.sh
│   ├── verify-plan-enrichment.sh
│   ├── verify-sensitive-audit.sh
│   └── iso/
├── testing/                   # CTest smokes (19 with event runtime OFF)
├── docs/os2.0/                 # phases, maturity, this tree
└── CMakeLists.txt
```

---

## Build flags

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DSOVEREIGN_BUILD_TESTS=ON \
  -DSOVEREIGN_BUILD_EVENT_RUNTIME=OFF   # default: quarantined
```

- `SOVEREIGN_BUILD_EVENT_RUNTIME=ON` — experimental `core/runtime` + extra tests  
- Product path does **not** link `sovereign_event_runtime`

---

## Milestone A hardening (done on main)

| ID | Commit | Result |
|----|--------|--------|
| A.1 | `f520a3b` | Session/principal revoke verified |
| A.2 | `09143a3` | Plan `enrichment=` + Ollama timeouts |
| A.3 | `a049861` | Audit on sensitive CLI paths |
| A.4 | `0173cff` | Event runtime quarantined (19 CTest) |
| A.5 | *(this doc)* | PROJECT_TREE refresher |

---

## Critical CLI surface

```
bootstrap / login / logout / session-revoke
operator-create / service-create / machine-create
principal-suspend / principal-revoke / change-credential
status / brief / inspect / audit-verify / audit-list
policy-check / policy-dry-run / policy-version / policy-rule-set
memory-add / memory-rank / memory-prune / plan
loop [--auto-approve] / recommend / approve / reject / execute
```

---

## Standing rules (owner)

1. Revoke ≠ disk wipe; only `$SOVEREIGN_DATA_ROOT`  
2. Disk install = explicit device + CONFIRM (never via `loop --auto-approve`)  
3. MFA (future) = step-up gate, not installer  
4. Deferred: `http_api`, `desktop`, `hermes_bridge`  
5. Local Ollama first  

See: `RECOMMENDATIONS_MFA_DISK.md`, `QUARANTINE_RUNTIME.md`, `MATURITY_MATRIX.md`

---

## How models should use this file

On context load, read in order:

1. `docs/os2.0/PROJECT_TREE.md` (this file)  
2. `docs/os2.0/MATURITY_MATRIX.md`  
3. `docs/os2.0/DAILY_USE.md`  
4. `docs/os2.0/RECOMMENDATIONS_MFA_DISK.md`  

Do **not** re-bootstrap design from zero. Extend existing modules; keep governance gates; quarantine does not mean delete.

**Next after A.5:** Milestone B — non-destructive VM installer design (no host disk write until device named).
