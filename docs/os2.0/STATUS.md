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
- L6 node <-> capability binding *(done; awaiting final merge to `main`)*

## In progress / Starting point
- **Active branch marker:** `leap/L7-governed-purpose-runner` (or `agent/<agent>/<timestamp>-L7-governed-purpose-runner`)
- **L7:** Governed purpose runner (C++) — route `purpose-run` through CapabilityGate + governance for HIGH actions and audit every allow/deny
