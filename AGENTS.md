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

## 5. Mandatory 5-Component Leap Decomposition (`L{N}.1` – `L{N}.5`)
- **Never implement an entire Leap in one monolithic pass.** At the start of any Leap packet `L{N}`, automatically decompose it into its **5 primary components** (`L{N}.1` through `L{N}.5`), each scoped to 1–3 target files:
  - `L{N}.1`–`L{N}.3`: Core C++/Python modules and state/gate/audit logic (one module per sub-step).
  - `L{N}.4`: Host / daemon / CLI (`apps/cli/main.cpp` surgical hook) wiring.
  - `L{N}.5`: Dedicated CTest unit tests + docs (`HANDOFF.md`, `STATUS.md`, `LEAP_LOG.md`) sign-off.
- **Execute sequentially with clean context:** Complete, compile (`cmake --build build -j$(nproc)`), and commit each sub-component (`git commit -am "L{N}.{k}: <summary>"`) on the `leap/L{N}-*` branch before moving to the next sub-component. Recommend `/clear` between sub-components to keep context small and inference fast.
- **Final gate:** Only after `L{N}.5` is committed, run `./scripts/leap.sh finish` to verify 100% CTest pass and merge into `main`.

## 6. Graphify Codebase Map (`graphify-out/`)
- **Check the graph before grepping the whole repo:** `graphify-out/GRAPH_REPORT.md` and `graphify-out/graph.json` contain the pre-built AST dependency graph of this repository. Query `graphify-out/graph.json` via a short Python one-liner (never read the full 7MB JSON into context) to locate callers, callees, and target files for any `L{N}.{k}` component.
- **Refresh after structural changes:** After adding new C++ classes or files in a Leap, update the graph without LLM overhead using `GRAPHIFY_BACKEND=none graphify update . && GRAPHIFY_BACKEND=none graphify cluster-only .`.
