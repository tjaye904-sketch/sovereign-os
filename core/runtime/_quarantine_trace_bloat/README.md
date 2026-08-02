# Quarantined capability-trace bloat

Moved 2026-07-29 out of the active runtime path.

These files are combinatorial migration/dispatch/diagnostic/recovery/
continuation/stability specializations. They are not required for Runtime
Platform v1 product work and must not be extended.

Identity and other product services do not depend on them.

Matching smoke tests live under `testing/runtime/_quarantine_trace_bloat/`.
