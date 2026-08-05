# Sovereign project tree — AI / model context refresher

**Updated:** 2026-08-05 (Milestones A–C + D.1/D.2 complete)  
**Repo root (host):** `~/src/sovereign-os/sovereign-work`  
**Data root:** `$SOVEREIGN_DATA_ROOT` or `~/.local/share/sovereign`  
**Git user:** tjaye904-sketch  

---

## What Sovereign is

Governance-first **local cognitive OS substrate**: identity → policy → memory → models (Ollama) → agents → audit.  
**Bootable NixOS live ISO** builds and QEMU-boots (D.2). Host disk install remains **gated**. MFA enroll/recovery works; not mandatory on every login.

| Scope | Status |
|-------|--------|
| Approved **v1 product** (CLI host) | ~100% |
| **OS 2.0** (bootable installable OS) | ~50–55% |
| Full life-OS vision | ~35–40% |

---

## Layout

```
sovereign-work/
├── apps/
│   ├── cli/main.cpp           # sovereign-cli (+ CmdTimer → metrics/cmd_times.log)
│   ├── daemon/main.cpp        # ServiceOrchestrator + agent/audit ticks
│   └── installer/             # sketch — no silent disk writes
├── core/
│   ├── identity/              # Argon2, TOTP, recovery codes, step-up
│   ├── governance/ policy/ memory/ cognitive/
│   ├── model_router/          # Ollama; curl max-time 25s
│   ├── audit/ os_runtime/ product/ diagnostics/
│   ├── agent/ goals/ briefing/ semantic/ domain/ workspace/
│   ├── archive/ sync/ executor/
│   └── runtime/               # EXPERIMENTAL — SOVEREIGN_BUILD_EVENT_RUNTIME=OFF default
├── nix/                       # flake ISO + sovereign-system + sovereign-vm
├── scripts/
│   ├── build-sovereign-nix-iso.sh  test-sovereign-iso-boot.sh
│   ├── build-sovereign-vm.sh run-sovereign-vm.sh
│   ├── iso/sovereign-install.sh sovereign-first-boot.sh
│   └── verify-*.sh
├── testing/                   # 23 product smokes (event runtime OFF)
├── dist/Sovereign-OS-nix-amd64.iso   # live image (rebuild as needed)
└── docs/os2.0/                 # phases, MATURITY_MATRIX, this tree
```

---

## Build flags

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DSOVEREIGN_BUILD_TESTS=ON \
  -DSOVEREIGN_BUILD_EVENT_RUNTIME=OFF
```

---

## Milestone commits (selected)

| ID | Topic |
|----|--------|
| A.* | revoke, enrichment, audit, quarantine, tree |
| B.* | installer gates, VM flake, first-boot |
| C.* | MFA scaffold, TOTP, recovery/step-up |
| Post-C.3 | agent Load fix, orchestrator smoke |
| D.1 | `cmd_times.log` + inspect timing |
| D.2 | ISO 1.4G + boot PASS |

---

## Critical CLI surface

```
bootstrap login logout session-revoke mfa-status mfa-enroll-* mfa-recovery-generate
status brief inspect audit-verify plan loop recommend approve reject execute [--mfa]
policy-* memory-* service-create machine-create
```

---

## Standing rules

1. Revoke ≠ disk wipe  
2. Disk install needs explicit device + CONFIRM (+ `SOVEREIGN_INSTALL_ALLOW_HOST=1`)  
3. `loop --auto-approve` never calls installer  
4. Deferred: http_api, desktop, hermes_bridge  
5. Local Ollama first  

See: `MATURITY_MATRIX.md`, `RECOMMENDATIONS_MFA_DISK.md`, `D2_ISO_EVIDENCE.md`, `PHASE_C_MFA.md`

---

## Model load order

1. `docs/os2.0/PROJECT_TREE.md` (this file)  
2. `docs/os2.0/MATURITY_MATRIX.md`  
3. `docs/os2.0/DAILY_USE.md`  
4. `docs/os2.0/D2_ISO_EVIDENCE.md`  

Do not re-bootstrap design from zero. Extend modules; keep governance gates.
