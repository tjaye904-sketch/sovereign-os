# L4 — Capability bridge (Python CU / sys → C++ gates)

## Goal
Host automation services must respect the same capability registry as the CLI.

## Design
- `capability_client.gate_check(action)` shells to `sovereign-cli capability-gate-check`
- Fail closed if CLI missing or capability disabled
- Inject at start of CU `dispatch()` (skip `ping` / `list_tools`)

## Acceptance
1. `host.computer_use` enabled → browser_open allowed by gate-check
2. disabled → `allowed=0` and CU returns `capability_denied`
3. C++ CTest still 100% (no C++ API break required)
4. Documented in HANDOFF / LEAP_LOG

## Branch
`leap/L4-capability-bridge`
