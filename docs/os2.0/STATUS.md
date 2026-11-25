## Branch policy
- **Never touch `main` directly:** Before doing anything on the repo, create an isolated agent session branch (`agent/{agent}/{YYYYMMDD-HHMM}-{topic}` via `scripts/leap.sh agent-start <agent> <topic>`).
- **One Leap packet (`L` packet) per branch:** Implement each `L` packet on its own `leap/L{N}-…` branch (`scripts/leap.sh start leap/L{N}-…`).
- **Strict merge gate:** Merge `leap/L{N}-…` into `main` only via `scripts/leap.sh finish` once stable and at **100% CTest pass** (`agent/*` branches never merge directly to `main`).

## Completed
- L1–L7: Capability Registry, Node Federation, CapabilityGate, Python CU/sys Bridge, Mission Control Caps Panel, Node-Capability Binding, Federation Peer Auth
- L8–L10: Governed Purpose Runner (`937dfc1`), Desktop Operator Visibility (`ca5109f`), 5-Service Session Ensure Ready (`d6c9343`)
- L11–L13: Federation Plan Routing (`0409941`), Federation Node Execute (`fcab1f9`), Mission Control Federation & Governance Console (`10ad708`)
- L14–L16: Core HA Backup & Restore (`6703fd5`), NixOS Lab & Headless Node Image Gates (`b89f13d`), Multi-Workspace Node Fleet Scale & Replace Drill (`132e59d`)
- L17–L18: Post-Act Outcome Verification & Closed-Loop Execution (`9ac9c8c`), Policy-Bounded Curiosity Probe (`leap/L18-curiosity-policy-bounded-probe`) — `CuriosityEngine::RunBoundedProbe`, `sovereign-cli curiosity-probe [--json]`, and Mission Control `/api/curiosity`

## In progress / Starting point
- **OS 2.0 Leaps L1–L18 complete on `main` (57/57 CTest)**
