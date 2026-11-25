# L6 — Node ↔ capability binding

## Goal
A node may only bind capability ids that exist in the CapabilityRegistry.
Unknown ids are rejected. Node list / heartbeat can report bound capability health via CapabilityGate.

## Acceptance
1. `node-caps-bind <node> <cap-id>` succeeds only if cap exists and (optionally) enabled
2. Unknown cap-id → clear error, no write
3. `node-caps-list <node>` lists bound ids
4. `node-caps-unbind <node> <cap-id>` removes binding
5. Smoke + CTest 100%
6. Branch: leap/L6-node-capability-binding

## Next batch (do not start until L6 merged)
- L7 Federation peer auth scaffold
- L8 Workspace purpose allowlist enforcement
- L9 Audit export + evidence bundle
- L10 Daily host session ensure-ready (daemon/CU/sys/hub/caps)
