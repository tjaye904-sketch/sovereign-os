# Sovereign OS 2.0 & JARVIS — Engineering Handoff

## Current Baseline (`main` @ `L28`)
- **Completed Foundation (`L1`–`L27`):** All Master Build Plan phases (`P0`–`P7`) and JARVIS Completion Pathway work packages (`WP-00`–`WP-13`) are merged on `main` and pushed to `origin/main` (`63/63` CTest passing).
- **Live Pop!_OS Host Session:**
  - `sovereign-daemon`, `sovereign-computer-use` (`:8765`), `sovereign-sys` (`:8767`, headed), `sovereign-hub` (`:8766`), `sovereign-hub-chat` (`:8768`), and `sovereign-caps-ui` (`:8769` + `/ui`, managed by `systemd --user`) are active.
  - `sovereign-cli doctor --fix` passes `6/6` hardening & readiness checks; `149` host `.desktop` applications are synced into the unified `PolicyEngine` allowlist; `sovereign-hub-chat` defaults to `chat-tools` with `qwen2.5-coder:7b`.
- **Release ISO Artifact (`WP-13` / `L26`):**
  - `~/src/sovereign-os/releases/sovereign-os-2.0-0f246fd-x86_64.iso` (`1.4G`, `SHA256SUMS` verified, GC-pinned, and QEMU serial-boot verified).

## Next-Horizon Build Plan (`L28` – `L31`)
1. **L28 (`leap/L28-systemd-caps-service-and-clean-build` — Current):** Zero-warning C++23 build (`apps/cli/main.cpp`), persistent `systemd --user` `sovereign-caps-ui.service` (`:8769`), and updated `HANDOFF.md` + OS 2.0 docs.
2. **L29 (`leap/L29-autonomous-multi-turn-tool-loop` — Done):** Autonomous multi-turn tool dispatch + synthesis loop inside `sovereign-cli chat-tools` (`list_tools`, `status`, `doctor`, `inventory_check`, `purpose_list`, `goal_execute`) bounded to max 4 turns with `CapabilityGate` enforcement.
3. **L30 (`leap/L30-live-a11y-window-and-piper-audio` — Next):** Live Linux window/input dispatch (`wmctrl` / `xdotool` / AT-SPI2 via `sovereign-sys`) for `DesktopOperatorGateway::DriveWindow` and live Piper TTS (`en_US-lessac-medium.onnx` + `pw-play`/`aplay`) for `RouteVoice` when headed/audio hardware is present.
4. **L31 (`leap/L31-multi-node-enrollment-and-usb-release` — Final Step):** Two-node peer enrollment drill (`scripts/sovereign-multinode-drill.sh` pairing a secondary node/VM with the Pop!_OS core over peer-auth tokens) and safe interactive USB flash & verification helper (`scripts/sovereign-flash-usb.sh`) for bare-metal / secondary-machine boot.
