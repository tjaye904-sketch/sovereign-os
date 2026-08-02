# Audit 002 — Test NDEBUG Strip and Subscription Inventory Failures

**Date:** 2026-07-29
**Status:** Accepted (findings)
**Scope:** sovereign-work sandbox only; original repo untouched

## Summary

1. Release/NDEBUG strips all assert() checks — Release "passes" do not mean checks ran.
2. Tests use assert(action()) as the only call site for mutating ops — under NDEBUG those actions never run → nullopt deref → SIGSEGV.
3. subscription_inventory_smoke_test: O0/O2 with asserts = exit 0; O0/O2 with -DNDEBUG = SIGSEGV (139). ASan -O2 with asserts = exit 0.

## Root cause (confirmed)

Example from subscription_inventory_smoke_test.cpp:
  assert(bus.Subscribe(first));
Under NDEBUG the entire statement is removed. Subscribe never runs. Later *first_id is UB.

## Findings

- F1 High: Release tests do not validate behavior
- F2 High: assert-as-control-flow anti-pattern in smoke tests
- F3 High: inventory smoke SIGSEGV under NDEBUG (test-side)
- F4 Info: production inventory not shown failing under ASan -O2 with asserts on

## Recommendations

1. Replace assert(side_effect) with always-on REQUIRE / if (!x) return 1;
2. CI must run a configuration where checks are active
3. Optionally -UNDEBUG on test targets
4. Scan all smoke tests for assert(bus. patterns
5. Do not block Identity; track as runtime test debt

## Decision

Test evidence quality downgraded until F1/F2 fixed. Identity may proceed in sovereign-work.
