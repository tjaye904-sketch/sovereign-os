# L3.1–L3.5 Capability gates

| Leap | Feature |
|------|---------|
| L3.1 | `CapabilityGate` API (`Require`, `RequireForAction`) |
| L3.2 | Map + gate computer-use / browser actions → `host.computer_use` |
| L3.3 | Map + gate sys / app_open → `host.sys` |
| L3.4 | Action→capability map (`execute`, `purpose-run`, `voice`, …) |
| L3.5 | CLI `capability-gate-check`; deny on `purpose-run` / `execute` when disabled |

## Action map

| Action | Capability id |
|--------|----------------|
| execute | host.execute |
| purpose-run | host.purpose |
| browser / cu | host.computer_use |
| app_open / sys | host.sys |
| speak / listen | host.voice |
| chat / plan | model.chat |

## Policy

- **Fail closed** if capability missing or disabled.
- Seed via `capability-seed` / `SeedDefaults`.
- Disable: `capability-disable host.execute`
- Check: `capability-gate-check execute` → `allowed=0|1`

Python computer-use / sys servers should call the same ids (env or HTTP check) in a later leap; this batch hardens the C++ control plane.
