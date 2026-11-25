# Sovereign OS — Agent Execution Rules

## 1. Mandatory Branching & Protection
- **`main` is strictly protected.** Never edit, build, or commit directly on `main`.
- **Session isolation (required before any work):** Immediately run `scripts/leap.sh agent-start <agent> <topic>` (creates `agent/{agent}/{YYYYMMDD-HHMM}-{topic}`) before modifying or creating any files.
- **Leap packets (`L` packets):** Implement each `L` packet on its own `leap/L{N}-{short-slug}` branch (`scripts/leap.sh start leap/L{N}-{short-slug}`).
- **Merge gate:** Only `leap/*` branches merge into `main`, and only via `scripts/leap.sh finish` after `cmake --build build` and `ctest --test-dir build --output-on-failure` pass 100% on both the branch and merged tree. Never merge `agent/*` branches directly to `main`.

## 2. Debugging & Failure Protocol (Root Cause First)
- **Never blind-patch a build or CTest failure.** When `cmake` or `ctest --output-on-failure` fails:
  1. Write a brief step-by-step **root-cause trace** (identifying the exact failing symbol, state transition, or file boundary) *before* editing any source or test files.
  2. Fix the underlying bug rather than weakening test assertions to make a failing test pass.
  3. If two patch attempts fail on the same bug, stop and inspect the full call chain across `core/` before writing a third edit.

## 3. Codebase Guardrails
- Keep `apps/cli/main.cpp` edits small and surgical; never bulk-delete or run `grep '<< "        << "' apps/cli/main.cpp` and leave matches.
- Keep all host automation headed (visible on-screen) and local-Ollama only.
- Current milestone: **L6** complete (pending merge to `main`); **L7** (`leap/L7-governed-purpose-runner`) is the active starting point.

## 4. Automatic Model & Subagent Routing
- **Main session (`qwen3.5:4b` — fast GPU driver):** Handle git branch commands (`scripts/leap.sh`), file searches, reading `HANDOFF.md` / `GRAPH_REPORT.md`, running `/graphify`, and executing `cmake` / `ctest` builds directly in the main session.
- **Delegated subagent (`delegate_task` -> `qwen3-coder:30b`):** Never write or patch C++ source/header files (`.cpp`, `.hpp`, `.h`, `CMakeLists.txt`) or diagnose compiler/CTest failures directly in the main session. Always call `delegate_task` with the exact target file paths, error logs, and `HANDOFF.md` constraints so the `qwen3-coder:30b` subagent performs the root-cause trace and surgical code edits in a clean context window.
