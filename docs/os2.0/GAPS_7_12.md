# Gaps 7–12 (after main merge fd18e2b)

| # | Gap | Deliverable in this pack |
|---|-----|---------------------------|
| 7 | Native cognitive CLI surface | `sovereign-cli` wrappers already; `cognitive-*` shims + optional main.cpp hooks |
| 8 | Wayland window list | `sovereign-desktop-probe` v2: gdbus/COSMic/wl-info soft paths |
| 9 | Purpose ↔ CU single path | `purpose-run-unified.sh` prefers CU server then C++ CLI |
| 10 | API gateway auth scaffold | docs + env gate script (token file); no breaking open-by-default without flag |
| 11 | Evolution operator path | `verify-evolution-path.sh` exercises sandbox smoke + CLI memory |
| 12 | Remote main divergence | document only — no force-push |

Excluded (still deferred): bare-metal install, Secure Boot signing keys.
