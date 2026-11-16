# Sovereign PROJECT_TREE (auto)

Generated: 2026-11-16T03:39:31-05:00
Branch: main
HEAD: e0913a4

## Top-level
```
total 22052
drwxr-xr-x 18 tjaye904 tjaye904     4096 Nov 16 02:40 .
drwxrwxr-x 12 tjaye904 tjaye904     4096 Sep 10 02:42 ..
drwxr-xr-x  4 tjaye904 tjaye904     4096 Sep 25 09:16 apps
-rw-r--r--  1 tjaye904 tjaye904      484 Sep 25 09:16 BOOT.md
-rw-r--r--  1 tjaye904 tjaye904     1287 Sep 25 09:16 BOOT.sh
drwxrwxr-x 11 tjaye904 tjaye904     4096 Nov 16 02:40 build
drwxr-xr-x  3 tjaye904 tjaye904     4096 Jul 29 03:54 build-debug
-rw-rw-r--  1 tjaye904 tjaye904      792 Aug 27 07:45 .clinerules
-rw-r--r--  1 tjaye904 tjaye904     1282 Sep 25 09:16 CMakeLists.txt
drwxr-xr-x 23 tjaye904 tjaye904     4096 Sep 25 09:16 core
drwxr-xr-x  2 tjaye904 tjaye904     4096 Sep 25 09:16 dist
drwxr-xr-x  7 tjaye904 tjaye904     4096 Nov 14 04:56 docs
-rw-r--r--  1 tjaye904 tjaye904      564 Sep 25 09:16 flake.lock
-rw-r--r--  1 tjaye904 tjaye904     2500 Sep 25 09:16 flake.nix
drwxrwxr-x 11 tjaye904 tjaye904     4096 Nov 16 03:36 .git
-rw-r--r--  1 tjaye904 tjaye904      470 Nov 14 04:56 .gitignore
drwxr-xr-x  5 tjaye904 tjaye904     4096 Sep 25 09:16 iso
-rwxrwxr-x  1 tjaye904 tjaye904      385 Aug 27 07:20 nano.499488.save
drwxr-xr-x  4 tjaye904 tjaye904     4096 Sep 25 09:16 nix
-rwxr-xr-x  1 tjaye904 tjaye904     5200 Nov 14 04:56 orchestrator.py
-rw-r--r--  1 tjaye904 tjaye904     1650 Sep 25 09:16 PROJECT_MANIFEST.yaml
-rw-rw-r--  1 tjaye904 tjaye904    72960 Aug  2 04:19 project_tree.txt
drwxr-xr-x  2 tjaye904 tjaye904     4096 Nov 14 04:56 purposes
-rwxr-xr-x  1 tjaye904 tjaye904     4379 Nov 14 04:56 quality_gate.py
-rw-rw-r--  1 tjaye904 tjaye904     3279 Nov 16 02:40 README.md
-rwxr-xr-x  1 tjaye904 tjaye904     3922 Nov 14 04:56 repo_scanner.py
-rw-r--r--  1 tjaye904 tjaye904      931 Sep 25 09:16 SANDBOX_POLICY.md
drwxr-xr-x  4 tjaye904 tjaye904     4096 Nov 16 03:38 scripts
-rw-r--r--  1 tjaye904 tjaye904      665 Nov 14 04:56 .sessions.json
-rw-r--r--  1 tjaye904 tjaye904 15269888 Aug 27 05:21 sovereign.qcow2
-rw-r--r--  1 tjaye904 tjaye904  7208960 Aug 26 21:38 sovereign-vm.qcow2
drwxr-xr-x  2 tjaye904 tjaye904     4096 Sep 25 09:16 systemd
drwxr-xr-x 20 tjaye904 tjaye904     4096 Nov 16 02:40 testing
drwxr-xr-x  5 tjaye904 tjaye904     4096 Nov 16 02:40 tools
drwxr-xr-x  5 tjaye904 tjaye904     4096 Nov 14 04:56 .venv
drwxr-xr-x  2 tjaye904 tjaye904     4096 Jul 30 08:32 verification-report
```

## core/
```
/home/tjaye904/src/sovereign-os/sovereign-work/core
/home/tjaye904/src/sovereign-os/sovereign-work/core/agent
/home/tjaye904/src/sovereign-os/sovereign-work/core/archive
/home/tjaye904/src/sovereign-os/sovereign-work/core/audit
/home/tjaye904/src/sovereign-os/sovereign-work/core/briefing
/home/tjaye904/src/sovereign-os/sovereign-work/core/cognitive
/home/tjaye904/src/sovereign-os/sovereign-work/core/common
/home/tjaye904/src/sovereign-os/sovereign-work/core/diagnostics
/home/tjaye904/src/sovereign-os/sovereign-work/core/domain
/home/tjaye904/src/sovereign-os/sovereign-work/core/executor
/home/tjaye904/src/sovereign-os/sovereign-work/core/goals
/home/tjaye904/src/sovereign-os/sovereign-work/core/governance
/home/tjaye904/src/sovereign-os/sovereign-work/core/identity
/home/tjaye904/src/sovereign-os/sovereign-work/core/memory
/home/tjaye904/src/sovereign-os/sovereign-work/core/model_router
/home/tjaye904/src/sovereign-os/sovereign-work/core/os_runtime
/home/tjaye904/src/sovereign-os/sovereign-work/core/policy
/home/tjaye904/src/sovereign-os/sovereign-work/core/product
/home/tjaye904/src/sovereign-os/sovereign-work/core/runtime
/home/tjaye904/src/sovereign-os/sovereign-work/core/runtime/_quarantine_trace_bloat
/home/tjaye904/src/sovereign-os/sovereign-work/core/semantic
/home/tjaye904/src/sovereign-os/sovereign-work/core/sync
/home/tjaye904/src/sovereign-os/sovereign-work/core/workspace
```

## apps/ scripts/ docs/os2.0/
```
cli
daemon
archive-worktree.sh
boot-installed-qcow2.sh
build-sovereign-cli.sh
build-sovereign-iso.sh
build-sovereign-nix-iso.sh
build-sovereign-vm.sh
chat-tools-bridge.sh
cognitive-cli-shims.sh
daily-operator-path.sh
e2e-temp-dataroot.sh
fix-plan-enrichment-timeouts.sh
gateway-auth-scaffold.sh
host-finish-all.sh
hub-api-actions.sh
install-purposes.sh
inventory-harden.sh
iso
merge-combined-to-main.sh
merge-to-main.sh
patch-hub-inventory.py
persist-sovereign-vm-disk.sh
purpose-run-unified.sh
__pycache__
run-all-smokes-25-30.sh
run-all-smokes-31-36.sh
run-all-tests.sh
run-sovereign-vm.sh
run-sovereign-vm-ssh.sh
session-login-setup.sh
session-tools-chain.sh
smoke-agents.sh
smoke-archive-memory.sh
smoke-code-assist.sh
smoke-domains.sh
smoke-full-host.sh
smoke-goals.sh
smoke-governed-loop.sh
smoke-inspect.sh
smoke-mfa-session.sh
smoke-models.sh
ALIGNMENT_JARVIS.md
ARCHIVE.md
BASELINE_20261104T073245Z.md
BLUEPRINT_AND_ALIGNMENT.md
COMPLETION_PATHWAY.md
D2_ISO_EVIDENCE.md
DAILY_HOST.md
DAILY_USE.md
EVIDENCE_PRIORITY_1_5.md
EVIDENCE_WP01.md
EVIDENCE_WP01.md.save
EVIDENCE_WP02.md
EVIDENCE_WP03.md
EVIDENCE_WP04.md
EVIDENCE_WP05.md
EVIDENCE_WP06.md
EVIDENCE_WP07.md
EVIDENCE_WP08_WP09.md
EVIDENCE_WP10.md
EVIDENCE_WP11.md
EVIDENCE_WP12_14.md
EVIDENCE_WP15_17.md
EVIDENCE_WP18_20.md
GAPS_19_24.md
GAPS_25_30.md
GAPS_31_36.md
GAPS_7_12.md
HARDENING.md
HERMES_ALIGNMENT.md
HOST_AUTOMATION.md
HOST_MATURITY_REMAINING.md
kernel
MATURITY_MATRIX.md
MATURITY_MATRIX_UPDATE.md
MATURITY_MATRIX_WP17.md
MFA_REVOCATION_AND_DISK.md
MILESTONE.md
NIX_ISO.md
OPERATOR_SURFACES.md
ORIGIN_MAIN_NOTE.md
```
