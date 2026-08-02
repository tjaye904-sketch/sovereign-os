# Quarantined runtime test material

- `CMakeLists.txt.trace_bloat.bak` — former 1200+ line CMake with invalid `/*` comments and
  capability-trace name chains.
- Active runtime tests are the foundational list in `testing/runtime/CMakeLists.txt`.
- Drift-generated `runtime_capability_trace_*` smoke sources remain in `testing/runtime/`
  but are **not** registered in CMake/CTest until individually rehabilitated.
