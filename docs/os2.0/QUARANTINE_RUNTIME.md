# Quarantine: experimental `core/runtime` event stack (A.4)

## Decision

The large **event-bus / capability-invocation** tree under `core/runtime/` is **not** on the v1 product path (`SovereignHost` / CLI / daemon).

- **Product path uses:** `core/os_runtime` (ServiceOrchestrator) + host services.
- **Default build:** `SOVEREIGN_BUILD_EVENT_RUNTIME=OFF` — does not compile `sovereign_event_runtime` or its smoke tests.
- **Optional:** `-DSOVEREIGN_BUILD_EVENT_RUNTIME=ON` to rebuild experimental stack + runtime CTests.

## Already quarantined sources

`core/runtime/_quarantine_trace_bloat/` — generated trace/migration header chain; **do not** re-link into product.

## Why

Reduces compile time, avoids shipping unreferenced capability-trace bulk, keeps governance-first host as the supported surface.

## Re-enable (experimental only)

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DSOVEREIGN_BUILD_TESTS=ON -DSOVEREIGN_BUILD_EVENT_RUNTIME=ON
cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure
```
