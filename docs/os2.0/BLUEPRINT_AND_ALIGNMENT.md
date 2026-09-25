# Sovereign OS — Blueprint Map & Alignment
**Project tree:** `sovereign-work` (host path: Pop!_OS + optional Nix substrate)  
**Document status:** Living blueprint — corrected for **general purpose-driven host OS automation**  
**Last update:** 2026-09-25

---

## 0. Critical correction (product intent)

TradingView (or any single site/app) is **only an example**, not the product definition.

### Canonical capability statement

> **Sovereign provides general, purpose-driven automation of the host OS surface — browsers, windows, and desktop programs — so it can open, read, drive, and coordinate them for whatever defined purpose the operator sets, while remaining observable in real time on the desktop.**

Automation is always under:

```text
Purpose → Identity → Governance → Policy → Execution → Verification → Audit → Memory
```

Headed (visible) UI is the default for operator monitoring. Headless is opt-in for servers only.

---

## 1. What Sovereign is (north star)

Sovereign OS is a **federated, governed, cognitive operating platform**. Long-term, NixOS may serve as a declarative Linux substrate; **Sovereign** owns identity, governance, policy, orchestration, memory, planning, model routing, audit, and execution of workspaces and services.

**Current shipping shape:** a **single-host** governed cognitive layer on Pop!_OS (CLI + daemon + hub + computer-use), with Nix ISO/VM experiments secondary.

```text
Operator purpose
       ↓
Federation Core (single-node today)
       ↓
Workspaces / agents / models
       ↓
Host OS automation runtime (browser + desktop + OS actions)
       ↓
Observable desktop + audit
```

**Invariant:** Nodes (and apps) provide *capability*, not *authority*. Authority stays in identity + governance + policy.

---

## 2. Blueprint map (architecture layers)

| Layer | Responsibility | Current alignment |
|-------|----------------|-------------------|
| L0 Substrate | Linux host or NixOS | **Partial** — daily = Pop!_OS; Nix experimental |
| L1 Boot / target | Boot into Sovereign control plane | **Designed / ISO experiments** — not daily |
| L2 Federation Core | Identity, governance, policy, memory, plan, schedule, workspaces, models, audit, secrets | **Partial–strong on one node** |
| L3 Runtime | Events, lifecycle, services, recovery | **Partial**; event stack often feature-flagged off |
| L4 Workspaces | Domain execution domains (up to 16) | **Partial shells** |
| L5 Intelligence | Model router, agents, tools | **Partial** (Ollama + bridges) |
| L6 **Host automation** | Purpose-driven browser/window/app control | **Partial** — browser strong; desktop weak |
| L7 Presence | Mission Control / hub, voice | **Partial** (hub v2) |
| L8 Multi-node federation | Membership, network, placement | **Designed only** |
| L9 Physical | Robotics / hardware federation | **Designed only** |

---

## 3. Federation Core (single-node product target)

| Component | Role | Status |
|-----------|------|--------|
| Identity | Users, services, machines, MFA, revoke | Partial–strong |
| Governance | Recommend / approve / reject / execute | Partial–strong |
| Policy | Allow/deny, versions, dry-run, risk | Partial–strong |
| Memory | Working + tiers toward constitutional | Partial |
| Planner | Goals → steps | Partial |
| Scheduler | Agent tick / daemon services | Partial |
| Workspace controller | Domain lifecycle | Partial |
| Model router | Local models, timeouts, observe route | Partial |
| Audit | Append-only evidence | Partial–strong |
| Secret manager | Keyring / restricted files | Weak–partial |

---

## 4. Host OS automation (corrected scope)

### 4.1 Purpose package

A **purpose** is a named, policy-bound intent, for example:

- `research.web_collect`
- `dev.review_repo`
- `ops.check_services`
- `life.inbox_triage`
- `finance.market_observe` *(example only)*

Each purpose declares:

| Field | Meaning |
|-------|---------|
| `purpose_id` | Stable name |
| `allowed_tools` | launch, snapshot, click, type, screenshot, os.notify, … |
| `allowed_apps` | desktop ids / binary names / URL host allowlists |
| `max_risk_auto` | low \| medium (never critical without approve) |
| `require_headed` | true for operator-visible runs |
| `success_criteria` | optional checks |

### 4.2 Driver capabilities (general)

| Capability | Description |
|------------|-------------|
| `app.launch` | Start app or URL via session (`xdg-open`, desktop file) |
| `browser.open` | Headed browser, persistent profile |
| `browser.snapshot` | DOM/text read |
| `browser.act` | click / type / key (selector-based) |
| `desktop.list_windows` | List visible windows |
| `desktop.focus` | Focus by title/class when platform allows |
| `desktop.snapshot` | Screenshot (window or full) |
| `desktop.input` | Key/mouse where safe (X11/a11y; Wayland-limited) |
| `os.notify` | User notification |
| `os.files_open` | Open path under `$HOME` only by default |

### 4.3 Observability

- Headed windows on the operator desktop  
- Mission Control / hub shows active purpose, last actions, CU health  
- Screenshots + JSONL audit under `SOVEREIGN_DATA_ROOT/computer_use/`  

### 4.4 Non-goals (explicit)

- Silent headless takeover of the session without operator awareness (default)  
- Ungoverned root or cross-user automation  
- Binding the architecture to one commercial website or broker  

---

## 5. Alignment summary (blueprint vs current tree)

| Theme | Score (approx.) | Notes |
|-------|-----------------|-------|
| Cognitive + governance (single host) | 65–75% | CLI/daemon real |
| Host automation (general) | 40–50% | Browser path works via long-lived server; desktop/a11y early |
| Presence (hub/voice) | 55–65% | Hub v2; voice optional |
| Nix bootable federation OS | 15–25% | Experiments, not daily driver |
| Multi-node federation | 5–15% | Design |
| 16 mature workspaces | 10–25% | Names/shells |
| **Overall vs full federation OS doc** | **~25–35%** | Host product higher (~70% of *host* scope) |

---

## 6. Milestone map (implementation order)

**Gate every milestone:** configure → build → full tests → smoke → doc evidence → commit → only then next.

| ID | Milestone | Outcome |
|----|-----------|---------|
| M0 | Baseline freeze | Clean Release + CTest on `sovereign-work` |
| M1 | Core single-node E2E | Identity→plan→approve→execute→audit→persist |
| M2 | Runtime flags safe | Event runtime optional; default product green |
| M3 | Memory tiers + capability manifest | Constitutional vs working; tool catalog |
| M4 | Workspaces depth (priority domains) | Real workflows, not empty labels |
| **M5** | **General host automation** | Purpose packages + drivers + hub wiring |
| M6 | Nix VM boot path | sovereign services in VM without host wipe |
| M7 | Multi-node | Two-node governed action |
| M8 | Controlled evolution | Dev-workspace patch loop |
| M9 | Physical | Only after M5–M7 basics |

### M5 breakdown (host automation — primary for this correction)

| ID | Work | Verify |
|----|------|--------|
| M5.0 | Purpose schema + allowlist engine | Unit tests |
| M5.1 | Long-lived driver session (server) default | Multi-step snapshot ≠ about:blank |
| M5.2 | Browser profile generic (any allowlisted host) | Open 2 different sites in one session |
| M5.3 | Desktop launch + screenshot + list windows | Smoke script |
| M5.4 | Hub: active purpose + action log | UI/API smoke |
| M5.5 | CLI/`chat-tools` bridge to purpose.run | One governed end-to-end purpose |
| M5.6 | Example purposes (research, dev, ops) — **not** vendor-locked | Docs + smokes |

---

## 7. Workspaces (16) — role vs automation

Workspaces **define purpose and policy context**.  
Host automation **executes UI/OS actions** those purposes require.

Examples:

| Workspace | May use automation for |
|-----------|-------------------------|
| Development | IDE/browser docs, CI pages |
| Knowledge | Reading documentation sites, local files |
| Infrastructure | System monitors, service UIs |
| Finance | *Optional* market UIs — not required for Core |
| … | Per policy |

---

## 8. Verification philosophy

```text
Generated ≠ Implemented
Implemented ≠ Verified
Verified build ≠ Verified runtime
Verified runtime = candidate for acceptance
```

Local machine (compiler, tests, headed smoke) is authoritative.

---

## 9. Document control

| Path | Role |
|------|------|
| `docs/os2.0/BLUEPRINT_AND_ALIGNMENT.md` | This file — master map |
| `docs/os2.0/HOST_AUTOMATION.md` | Purpose-driven automation spec |
| `docs/os2.0/MATURITY_MATRIX.md` | Update % from this alignment |
| `PROJECT_TREE` / refresher | Keep in sync after each milestone |

**End of BLUEPRINT_AND_ALIGNMENT.md**
