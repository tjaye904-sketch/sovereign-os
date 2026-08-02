# OS 2.0 Completion Evidence Chain

| Requirement | Spec | Implementation | Tests | Observable | Docs |
|-------------|------|----------------|-------|------------|------|
| Boot menu modes | PHASE_I_BOOT | iso/grub/grub.cfg | manual ISO | boot menu | PHASE_I_BOOT |
| Boot diagnostics | PHASE_I_BOOT | scripts/boot/sovereign-boot-diag.sh | host run | script output | PHASE_I_BOOT |
| Linux kernel path | kernel/config-sovereign | ISO debootstrap | ISO build host | uname | config-sovereign.md |
| Service dependency order | PHASE_II_RUNTIME | ServiceOrchestrator | service_orchestrator_smoke_test | state list | PHASE_II_RUNTIME |
| Watchdog restart | PHASE_II_RUNTIME | WatchdogTick | smoke test | restarts counter | PHASE_II_RUNTIME |
| v1 identity…transparency | v1 specs | core/* | CTest 33 | inspect/ask | SPEC_EVIDENCE_MATRIX |
