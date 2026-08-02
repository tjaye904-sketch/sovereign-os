# Quality Gates (2026-07-30)

## Clean-build defects fixed

1. **`testing/runtime/CMakeLists.txt` parse error** — invalid `/* */` C-style comments and trace-bloat chain; replaced with foundational smoke list; old file quarantined under `testing/runtime/_quarantine/`.
2. **`sovereign_executor` missing `policy_engine.hpp`** — linked `sovereign_policy` in executor CMakeLists.
3. **Diagnostics/product link order** — `core/diagnostics` added *after* `core/product`; CLI links `sovereign_diagnostics`.

## Shared test utility

`testing/common/test_require.hpp` — always-on `REQUIRE` (not stripped by NDEBUG).

## CTest

33 tests registered (product + runtime foundational + CLI/daemon parse). **Release: 33/33 pass.**

## Sanitizers

`scripts/run-all-tests.sh ASan|UBSan` — ASan smoke on identity/governance: pass.

## E2E

`scripts/e2e-temp-dataroot.sh` — bootstrap→…→execute→inspect→persistence: **OK**.

## CI

`.github/workflows/clean-zip-build.yml` — zip extract + cmake + ctest + e2e.

## Spec evidence

`docs/project/SPEC_EVIDENCE_MATRIX.md`
