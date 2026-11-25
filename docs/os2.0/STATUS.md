## Branch policy
- **Never touch `main` directly:** Before doing anything on the repo, create an isolated agent session branch (`agent/{agent}/{YYYYMMDD-HHMM}-{topic}` via `scripts/leap.sh agent-start <agent> <topic>`).
- **One Leap packet (`L` packet) per branch:** Implement each `L` packet on its own `leap/L{N}-…` branch (`scripts/leap.sh start leap/L{N}-…`).
- **Strict merge gate:** Merge `leap/L{N}-…` into `main` only via `scripts/leap.sh finish` once stable and at **100% CTest pass** (`agent/*` branches never merge directly to `main`).

## Completed
- L1 capability registry
- L2 node federation
- L3 CapabilityGate + gate-check (`e87a8f7`)
- L4 capability bridge (Python CU/sys)
- L5 Mission Control capability panel
- L6 node <-> capability binding (`7da20bb`)
- L7 federation peer auth (`1a58ad2`)
- L8 governed purpose runner (`937dfc1`)
- L9 desktop operator visibility (`ba94a88d`) — `DesktopOperatorStatus` telemetry, `ApiGatewayService::GetDesktopStatus()`, `sovereign-cli desktop-status [--json]`, and Mission Control `/api/desktop-status`

## In progress / Starting point
- **OS 2.0 Capability, Federation, Governance & Operator Visibility Leaps (L1–L9) complete on `main` (48/48 CTest)**
