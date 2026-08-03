# Evaluation Report — 2026-08-02

**Scope:** Full project-health verification + code review of the `sovereign-cli loop`
command (commit 797b1ea, "Cognitive loop: plan → recommend → approve/execute → reflect").

**Method:** Fresh build, full test matrix (Release/ASan/UBSan), offline smoke, live
`loop` exercises on temporary data roots against a live local Ollama, and claim
cross-check against `PROJECT_MANIFEST.yaml`, `PROJECT_STATUS.md`,
`IMPLEMENTATION_MATRIX.md`, and `verification-report/`.

---

## Part A — Verification results

| Check | Result | Notes |
|---|---|---|
| CLI build (`scripts/build-sovereign-cli.sh`) | **PASS** | `loop` compiles; appears in usage |
| Daemon build (same script) | **FAIL** | Script bug, see Script-1 below (pre-existing) |
| Release ctest | **33/33 PASS** | e2e-temp-dataroot OK |
| ASan | **33/33 PASS** | e2e OK |
| UBSan | **33/33 PASS** | e2e OK |
| `scripts/smoke-v1-complete.sh` | **PASS** | Offline core path |
| `loop` paused path (owner) | **PASS** | plan→recommend→pause; approve+execute resume works |
| `loop --auto-approve` (operator) | **WORKS — but bypasses governance** | See Gov-1 |
| Ollama plan enrichment | **NOT triggered** | See Enrich-1 |

---

## Part B — `loop` command review

### Gov-1 (High) — `--auto-approve` grants Owner authority to any operator

`apps/cli/main.cpp:683` calls the legacy bool API:

```cpp
host.Governance().Approve(actor, /*owner_is_owner_role=*/true, did, "loop --auto-approve");
```

`governance_service.cpp:420-431` maps `true` → `GovernanceRole::Owner`, bypassing the
per-action `minimum_approval_authority_role` check in `Approve` (line 366). **Verified
live:** an `operator` principal ran `loop --auto-approve "ship release candidate"` and
the decision (default rule `mature` requires Owner approval, `governance_rule.hpp:19-25`)
was approved and executed, with a `loop_complete` audit entry.

Pre-existing companion flaw: the standalone `approve`/`reject` commands fabricate an
Owner principal and take no session at all (`apps/cli/main.cpp:829-831`,
`owner.bytes[0]=1`), so this bypass pattern already existed in the CLI; `loop`
extends it to authenticated operators.

### Loop-1 (Medium) — plan is decorative

The `ExecutionPlan` (including Ollama enrichment) is built and printed
(`apps/cli/main.cpp:637-648`) but never used to drive action selection. The action is
hardcoded to `"mature"` (line 650). The "plan → execute" link in the commit message is
nominal: plan steps do not influence what is recommended or executed.

### Loop-2 (Medium) — execution is a stub, and action↔decision not bound

The auto-approve path registers an in-process handler that returns
`"loop-executed:" + action` (`apps/cli/main.cpp:693-697`); the standalone `execute`
command likewise returns `"executed:" + action` (`apps/cli/main.cpp:1342-1346`). No real
work is performed. Additionally, `ActionExecutor::Execute` gate is only
`cognitive_.AssertApproved` (`action_executor.cpp:33-38`), which checks
`status == Approved` but never verifies the decision's stored `action` matches the
executed action — so `execute <anything> <any-approved-hex>` succeeds.

### Loop-3 (Low) — policy gate skipped

`Executor().Execute(action, decision_id)` is called without `actor_roles`, so the
PolicyEngine check is skipped entirely (`action_executor.cpp:40-49`, gated on
`!actor_roles.empty()`).

### Loop-4 (Low) — no automated test coverage

ctest registers only `cli_parse_usage` and `cli_parse_status` for the CLI
(`testing/CMakeLists.txt:36-44`). The 135-line `loop` command has zero test coverage;
its behavior is only observable manually.

### Loop-5 (Low) — paused/resume path gaps

The paused path (`apps/cli/main.cpp:663-678`) prints resume commands but never records a
reflection after the user resumes via `approve` + `execute` — the cognitive loop is only
completed end-to-end (reflect) in `--auto-approve` mode. It also does not re-validate
that the decision is still `Pending` before instructing resume, and swallows audit
append errors via `static_cast<void>`.

### Enrich-1 (Medium) — Ollama plan enrichment silently no-ops on thinking models

`EnrichPlanWithModel` (`cognitive_service.cpp:241-329`) calls `models_->Chat` and aborts
on empty content (line 266). `OllamaClient::Chat` extracts content via
`ExtractStringField(raw.value, "content")` (`ollama_client.cpp:261`), which returns the
**first** `"content"` key. With the installed `lfm2.5-thinking` model the `/api/chat`
response has `"content":""` (text lives in `message.thinking`), so enrichment silently
returns and plans stay heuristic — no `[model-enriched]` suffix, no diagnostic. Verified
live with `SOVEREIGN_OLLAMA_MODEL=lfm2.5-thinking:latest` while `status` reports
`ollama=up`.

### Script-1 (Medium) — daemon build broken in `scripts/build-sovereign-cli.sh`

Lines 72, 78, and 82 lack trailing `\`, splitting the daemon compile into fragments and
causing `undefined reference to main` / missing service symbols. The CLI build is
unaffected. Pre-existing; not introduced by the `loop` commit.

---

## Part C — Documentation consistency

| Document | Claim | Observed |
|---|---|---|
| `PROJECT_MANIFEST.yaml` | 100/100 tests passed | Current ctest = 33/33 (33 registered) |
| `PROJECT_STATUS.md` (2026-07-29) | Most product services "Not implemented" | Contradicts `IMPLEMENTATION_MATRIX.md` (2026-07-30), which marks them Implemented/Validated |
| `IMPLEMENTATION_MATRIX.md` | `sovereign-daemon` Implemented | Daemon does not build from `build-sovereign-cli.sh` (Script-1) |
| README / `OLLAMA_INTEGRATION.md` / `HOST_BUILD.md` | Default model `openclaw` (override `SOVEREIGN_OLLAMA_MODEL`) | Host has no `openclaw`; available: `lfm2.5-thinking`, `qwen3.6-large-ctx`, `qwen3-opencode` |

The manifest's 100/100 figure derives from Audit-001 (runtime-only era) and is stale as
a repository-wide test count. `PROJECT_STATUS.md` is superseded by the 2026-07-30
`IMPLEMENTATION_MATRIX.md`.

---

## Verdict

The runtime platform claims are substantiated: fresh Release/ASan/UBSan builds are all
green (33/33 + e2e) and the offline smoke passes. The new `loop` command compiles and its
happy path executes, but it is not yet a governed loop in the product sense:

1. `--auto-approve` is a governance bypass (any operator = Owner). Use the role-aware
   `Approve(actor, actor_role, …)` with the actor's real role resolved from the session
   principal, or route auto-approval through policy so that only rules permit it.
2. Plan→execute is cosmetic; action selection is hardcoded; execution is a stub.
3. Decision/action binding is absent (`AssertApproved` checks status only).
4. Model enrichment silently fails for thinking-family models.
5. `loop` has no automated tests.

**Recommended next steps:** (a) add a `loop` CLI test to `testing/` covering paused +
auto-approve paths; (b) fix the auto-approve role handling; (c) bind execute to the
approved decision's action; (d) make `EnrichPlanWithModel` tolerant of `thinking`
content and report enrichment failures; (e) fix the daemon build backslashes; (f)
reconcile `PROJECT_MANIFEST.yaml`/`PROJECT_STATUS.md` with the current matrix.
