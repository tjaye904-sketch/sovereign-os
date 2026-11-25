# Sovereign OS 2.0

A local-first, governed, multi-node C++23 & NixOS operating environment with cryptographic audit chains, capability gating, unified Mission Control UI, autonomous local Ollama tool routing, desktop/window control, Piper neural TTS, and hybrid EFI/BIOS live ISO & headless node VM builds.

## Core Architecture
- **C++23 Core Runtime (`core/`, `apps/daemon/`, `apps/cli/`):**
  - **Governance & Capability Gate (`CapabilityRegistry`, `CapabilityGate`, `PolicyEngine`):** Action-level capability enforcement, approval queues, unified desktop application inventory allowlist, and hash-chained tamper-evident `AuditLog`.
  - **Multi-Node Federation (`NodeRegistry`, `PeerAuth`, `PlanRouteDecision`):** Token-authenticated peer node enrollment, capability-aware workspace plan routing, and remote governed execution.
  - **Closed-Loop JARVIS Runtime (`PurposeRuntime`, `ActionExecutor`, `CuriosityEngine`, `DesktopOperatorGateway`):** Seeded purpose packs, post-act FNV-1a verification receipts, policy-bounded curiosity probes, live `wmctrl` window/A11y drive, and local Piper TTS (`en_US-lessac-medium.onnx`) audio routing.
  - **Autonomous Local Model Router (`ModelRouter`, `sovereign-cli chat-tools`):** Multi-turn tool dispatch and synthesis loop backed by local Ollama models.
- **Unified Mission Control (`tools/mission_control/`):** Live dashboard (`http://localhost:8769/ui`) for Doctor hardening diagnostics, governance approvals, federation & fleet status, desktop/voice telemetry, and capability toggles.
- **NixOS Flake (`flake.nix`, `nix/modules/`):** Reproducible `.#iso` (hybrid UEFI/BIOS live ISO) and `.#sovereign-node-vm` (hardened headless QEMU/KVM worker node).

## Quickstart (Linux Host Build & Test)
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j\$(nproc)
ctest --test-dir build --output-on-failure
```

## Run Host Services & Mission Control
```bash
./scripts/sovereign-session-ensure.sh
./build/apps/cli/sovereign-cli doctor --fix
# Open http://localhost:8769/ui in your browser
```

## Build Bootable NixOS ISO or Node VM
```bash
nix build .#iso
nix build .#sovereign-node-vm
```
Or download the prebuilt `sovereign-os-2.0-0f246fd-x86_64.iso` from **GitHub Releases**, verify `SHA256SUMS`, and write it to a USB drive or SD card using `./scripts/sovereign-flash-usb.sh --yes-flash /dev/sdX`.
