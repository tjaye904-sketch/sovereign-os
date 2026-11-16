# Specification coverage (host snapshot)

Generated: 2026-11-16T03:34:30-05:00

```
=== Sovereign transparency report ===

## current identity and active authority
  data_root="/home/tjaye904/.local/share/sovereign"
  surface=cli-only authority_model=governance+policy
  note=session_principal_from_login; owner highest rank
  roles=guest<operator<administrator<owner
  argon2=t=3 m_kib=262144 p=4

## loaded memory and provenance
  memory_records=8
  - tags=[cognitive,planning] body=goal:Short goal: list three host tasks|confidence=0.740000
  - tags=[cognitive,planning] body=goal:roadmap verify|confidence=0.740000
  - tags=[evolution] body=operator path verified 2026-11-16T03:24:53-05:00
  - tags=[evolution] body=operator path verified 2026-11-16T03:30:29-05:00
  - tags=[cognitive,planning] body=goal:Produce a structured plan with inspect then recommend for host health|confi
  semantic_records=0
  provenance=append-only memory + semantic embed_model + audit

## selected model/provider and routing reason
  chat_model=tripolskypetr/qwen3.5-uncensored-aggressive:4b
  chat_provider=ollama base=http://127.0.0.1:11434
  embed_model=granite-embedding:latest (fallback hash-v1 if unreachable)
  ollama=up
  routing_reason=owner_env SOVEREIGN_OLLAMA_MODEL or default; local-first only

## policy evaluations (sample)
  role=owner action=configure effect=allow reason=owner role
  role=owner action=deploy effect=allow reason=owner role
  role=owner action=read effect=allow reason=owner role
  role=operator action=configure effect=allow reason=operator role
  role=operator action=deploy effect=deny reason=rule deny deploy needs owner
  role=operator action=read effect=allow reason=operator role
  role=readonly action=configure effect=deny reason=readonly role
  role=readonly action=deploy effect=deny reason=rule deny deploy needs owner
  role=readonly action=read effect=deny reason=readonly role

## planned actions (pending governance)
  (none)
  active_goals=0

## approvals and denials (recent audit)
  (none recorded)

## executed commands
  (none recorded in audit)

## state changes
  workspaces=0
  goals_total=0
  memory_total=8
  domains=development,knowledge,business,finance,health,infrastructure,life,

## timing and resource usage
  process_uptime_s=0
  argon2=t=3 m_kib=262144 p=4
  cmd_samples=104 (last 12)
  cmd_wall 1794813423 chat 18584
  cmd_wall 1794814371 status 5
  cmd_wall 1794814383 chat 12356
  cmd_wall 1794814418 plan 9675
  cmd_wall 1794817493 memory-add 4
  cmd_wall 1794817829 memory-add 4
  cmd_wall 1794818070 plan 14258
  cmd_wall 1794818070 login 1
  cmd_wall 1794818070 loop 0
  cmd_wall 1794818070 peer-export 0
  cmd_wall 1794818070 audit-verify 0
  cmd_wall 1794818070 audit-list 0

## errors and recovery actions
  ollama=ok
  recovery_policy=fail-closed_governance; deny_by_default_policy
  peer_import=no_auto_execute

## immutable audit evidence
  entries=15 (tail)
  1794751523 identity/bootstrap_owner owner created
  1794751523 identity/login session issued
  1794751586 cognitive/plan WP21 verify enrichment honesty
  1794751746 daemon/stop orchestrator offline
  1794751771 daemon/start orchestrator online
  1794751799 daemon/stop orchestrator offline
  1794808477 daemon/start orchestrator online
  1794810463 daemon/stop orchestrator offline
  1794810463 daemon/start orchestrator online
  1794810534 cognitive/plan Host finish enrichment probe 1794810463
  1794811090 cognitive/plan Short goal: list three host tasks
  1794812866 cognitive/plan Short goal: list three host tasks
  1794812875 cognitive/plan Short goal: list three host tasks
  1794814418 cognitive/plan roadmap verify
  1794818070 cognitive/plan Produce a structured plan with inspect then recommend for host health
  store=append-only file audit.dat under data_root/audit

## subsystem health
  identity=open
  governance=open
  policy=open
  memory=open
  semantic=open
  workspace=open
  archive=open
  sync=peer-bundle
  goals=open
  audit=open
  agent_scheduler=open
  model_router=up
  daemon=external (sovereign-ctl status)

## specification coverage (approved v1)
  identity=complete governance=complete policy=complete
  memory=complete semantic=complete workspace=complete
  archive=complete sync_peer=complete goals=complete
  briefing=complete audit=complete domain_agents=complete
  daemon=complete cli=complete transparency=this_report
  deferred_by_owner=http_api,desktop,hermes_bridge
  approved_v1_scope=100%
```

## Automated tests
- CTest product suite: run `ctest --test-dir build`
- E2E: `sovereign-e2e-host`
