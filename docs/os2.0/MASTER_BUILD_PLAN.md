# Sovereign OS — Master Build Plan (ordered, anti-drift)

**Status:** binding process document for `sovereign-work`  
**Updated:** 2026-11-17  
**Profiles:** `host-pop` (daily operator) · `nixos-sovereign` (lab / node images)

---

## 0. North star (do not dilute)

1. **Sovereign is the Jarvis-style admin across the whole federation** — one cognitive + governance + operator plane, not 16 independent AIs.
2. **Each workspace gets its own headless node** (dedicated resources). Nodes are replaceable; workspaces are logical missions bound to nodes.
3. **Sovereign must not live only on one core box** — identity, policy, audit, bindings, and constitutional memory must be backup/failover capable so the OS *persists* when nodes die.
4. **NixOS is substrate; Sovereign is authority.** Host Pop!_OS remains the safe daily console until `nixos-sovereign` lab gates are green.
5. **Governed autonomy** — consequential actions need policy + human approval; fail closed when Core is unreachable.
6. **Evidence chain** — Requirement → Spec → Implementation → Tests → Observable run → Docs. No “done” without that chain for the active leap.

### Explicit non-goals (until later phases say otherwise)

- Bare-metal install that wipes the only laptop disk  
- 16 always-on full VMs on day one  
- Enabling experimental event runtime by default  
- Multi-cloud model routing as a blocker  
- Robotics / Ceph / full k8s before Core + 2-node proof  

---

## 1. How we work (process rules — anti-drift)

### 1.1 One active leap at a time

- Only **one** leap ID is `IN_PROGRESS`.  
- No starting leap N+1 until leap N exit criteria are checked and **git committed**.  
- Spike experiments go on a branch; merge only if they map to the current leap.

### 1.2 Definition of done (every leap)

| Gate | Required |
|------|----------|
| G1 Build | `cmake --build` (or pack install) succeeds |
| G2 Test | Relevant CTest and/or leap smoke script PASS (SOFT only where labeled) |
| G3 Observe | One operator command path demonstrated |
| G4 Docs | Leap notes in `docs/os2.0/LEAP_LOG.md` (append 5–10 lines) |
| G5 Commit | Single purposeful commit on `main` (or PR → main) |
| G6 Registry | Update `COMPONENT_REGISTRY` status for touched IDs |

### 1.3 Drift control artifacts (always current)

| Artifact | Purpose |
|----------|---------|
| `MASTER_BUILD_PLAN.md` | This file — order of work |
| `COMPONENT_REGISTRY_SEED.yaml` → `COMPONENT_REGISTRY.yaml` | I/V/G/S/D/P per component |
| `LEAP_LOG.md` | Append-only what shipped |
| `BLUEPRINT_ALIGNMENT_*.md` | Blueprint vs repo truth |
| `HOST_SCORECARD.md` | Live host health (optional each leap) |

### 1.4 Decision log

Any change to north star or leap order requires a one-line entry in `LEAP_LOG.md` under **DECISIONS** with date + reason. No silent scope swaps.

---

## 2. Architecture targets (fixed design)

```text
You (admin)
    ↓
Sovereign operator plane (CLI / Mission Control / voice)  ← Jarvis surface
    ↓
Federation Core (identity, governance, policy, memory, plan, audit, router, secrets, registry)
    ↓
Node agents (every node) ← heartbeat, policy citizen, local workspace runtime
    ↓
Workspace nodes (headless): 1 workspace ↔ 1 node (resources isolated)
```

**Persistence:** Core state backup + peer export; replaceable nodes re-bind; fail closed on partition for high-risk actions.

**Dual profile:**

| Profile | Role |
|---------|------|
| `host-pop` | Your laptop: operator + current Core home + CU/hub |
| `nixos-sovereign` | Image/lab for workspace nodes + future boot path |

---

## 3. Phase map (big blocks)

| Phase | Name | Outcome |
|-------|------|---------|
| **P0** | Freeze truth | Docs, registry, leap log, dual profile written |
| **P1** | Core hardening | Secrets, workspace controller, capability registry, memory facets |
| **P2** | Node fabric | Registry, bind, agent, one headless node, replace drill |
| **P3** | Jarvis federation UX | Route plans to nodes; MC shows federation; governance UI |
| **P4** | Workspace depth | Dev → Knowledge → Security → Research… (depth-first) |
| **P5** | Core HA / persist | Backup, restore, standby or cold failover |
| **P6** | NixOS lab maturity | ISO/VM gates automated; node image from flake |
| **P7** | Scale | More nodes on demand toward 16; optional mesh |

**Never reorder phases** without a DECISIONS log entry. Within a phase, leaps are sequential.

---

## 4. Detailed leap backlog (execute in order)

### P0 — Freeze truth

| Leap | Title | Work | Exit criteria |
|------|-------|------|----------------|
| **L0.1** | Master plan in repo | Install this doc + LEAP_LOG skeleton | Files on `main` |
| **L0.2** | Registry live | Promote seed → `COMPONENT_REGISTRY.yaml`; 16 workspace stubs listed | YAML committed; 16 names present |
| **L0.3** | Dual-profile doc | `PROFILES.md` host-pop vs nixos-sovereign | Doc committed |

### P1 — Federation Core (highest leverage)

| Leap | Title | Work | Exit criteria |
|------|-------|------|----------------|
| **L1.1** | Secret Manager MVP | Core store/lookup (file or keyring), audit on access, CLI | CTest or smoke + audit line |
| **L1.2** | Memory facets | Namespaces: constitutional / working / decision tags; inspect shows facets | inspect smoke |
| **L1.3** | Workspace Controller MVP | list / activate / suspend API + CLI (logical, may not start VMs yet) | CTest + CLI |
| **L1.4** | Capability registry | JSON registry + list CLI; policy can reference capability id | smoke |

### P2 — Node fabric (workspace = headless computer)

| Leap | Title | Work | Exit criteria |
|------|-------|------|----------------|
| **L2.1** | Node registry + bind | `node-register`, `node-list`, `node-bind <workspace> <node>` | CLI + test; registry file/DB under data_root |
| **L2.2** | Node agent v0 | Heartbeat process package; reports to Core | Heartbeat visible in inspect or log |
| **L2.3** | First headless node | One QEMU **or** nspawn node for `development` | SSH or agent health from host |
| **L2.4** | Controller starts node | activate development → node up (scripted) | Observable start/stop |
| **L2.5** | Replace drill | Destroy lab node → new node → rebind | Documented smoke PASS |
| **L2.6** | Second workspace node | e.g. `research` + isolation note | Two nodes listed; separate data dirs |

### P3 — Jarvis across federation

| Leap | Title | Work | Exit criteria |
|------|-------|------|----------------|
| **L3.1** | Plan step routing | Plan tags `workspace`/`node`; dry-run route table | CLI shows route |
| **L3.2** | Execute via agent | One governed action runs on bound node | Audit has node id |
| **L3.3** | MC federation panel | Nodes + binds + health in Mission Control | UI smoke |
| **L3.4** | Governance console | Approve/reject pending from UI | UI + audit |

### P4 — Workspace depth (not 16 UIs first)

| Leap | Title | Work | Exit criteria |
|------|-------|------|----------------|
| **L4.1** | Development tools | Non-stub tool path (e.g. project-summary/code-assist wired) | Test |
| **L4.2** | Knowledge ingest | File/url → semantic store | Query smoke |
| **L4.3** | Security thin WS | Audit query + policy dry-run as workspace surface | Smoke |
| **L4.4** | Research formalization | Purposes bound to research node profile | Purpose smoke |
| **L4.5+** | Next domains | business/finance/life/health… **one leap each** | Same DoD |

### P5 — Persistence / HA

| Leap | Title | Work | Exit criteria |
|------|-------|------|----------------|
| **L5.1** | Scheduled Core backup | peer-export or snapshot timer | Artifact on disk |
| **L5.2** | Restore drill | New data_root from backup | Login + policy version OK |
| **L5.3** | Fail-closed policy | High-risk denied if Core unreachable (node agent) | Smoke |
| **L5.4** | Cold standby Core doc+script | Second location restore | Written + dry-run |

### P6 — NixOS lab

| Leap | Title | Work | Exit criteria |
|------|-------|------|----------------|
| **L6.1** | One-command lab gate | flake eval/build ISO or VM + boot markers script | Script PASS |
| **L6.2** | Node image from flake | Headless image used in L2.3+ | Image boots agent |
| **L6.3** | sovereign.target notes | Map lab services to blueprint target | Doc only until real |

### P7 — Scale (only after P2–P5 proofs)

| Leap | Title | Work | Exit criteria |
|------|-------|------|----------------|
| **L7.x** | On-demand nodes | Create node on workspace activate | Not 16 always-on |
| **L7.y** | Mesh optional | WireGuard/Tailscale **if** multi-machine | Optional |

---

## 5. Parallelism rules (what may overlap)

| Allowed in parallel | Not allowed |
|---------------------|-------------|
| Doc typos / registry status fixes | New workspace UI while L1.x open |
| Host smoke scripts that don’t change Core APIs | Enabling full event runtime |
| Ollama model swaps | Bare-metal install experiments mid-leap |
| CU purpose YAML for **already** formalized workspace | Redesigning identity from scratch |

Operator laptop fixes (daemon autostart, hub) may ship as **hotfix leaps** `H.x` with same DoD, logged in LEAP_LOG, without changing phase order.

---

## 6. Weekly rhythm (high yield)

1. Read **next leap only** in §4.  
2. Implement **only** that leap’s work.  
3. Run G1–G6.  
4. Commit. Append LEAP_LOG.  
5. Stop or take **one** hotfix H.x if host is broken.  
6. Do not “while we’re here” into the next leap.

---

## 7. Current position (update when leaps complete)

| Item | State at plan adoption |
|------|-------------------------|
| Host product (CLI, daemon, plan, CU, MC) | Strong — treat as P0 baseline |
| Blueprint alignment docs | Install alongside this plan |
| Node fabric | **Not started** → begin after P0 → P1 minimum L1.1–L1.3 recommended before L2.3 |
| 16 workspace nodes | Design only |
| Core HA | Peer export exists directionally; not automated HA |

**First execution pointer after adopting this plan:** complete **L0.1 → L0.3**, then **L1.1**.

---

## 8. Scoreboard (update monthly)

| Track | Target | Measure |
|-------|--------|---------|
| Core | L1.1–L1.4 done | Registry statuses V |
| Nodes | L2.1–L2.5 done | Replace drill PASS |
| Jarvis | L3.1–L3.4 done | Audit shows node id |
| Depth | ≥3 workspaces non-stub | L4.x commits |
| Persist | L5.1–L5.2 done | Restore drill PASS |

---

## 9. One-page checklist (print / pin)

```text
[ ] Only one leap IN_PROGRESS
[ ] Exit criteria written before coding
[ ] Tests/smokes run
[ ] LEAP_LOG appended
[ ] COMPONENT_REGISTRY updated
[ ] Commit on main
[ ] No bare-metal wipe
[ ] No skipping to P4/P7 early
[ ] Sovereign = federation Jarvis, not single-node chatbot
[ ] Workspace = headless node binding
[ ] Core state backup considered for any Core change
```
