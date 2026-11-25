## Branch policy
- **Never touch `main` directly:** Before doing anything on the repo, create an isolated agent session branch (`agent/{agent}/{YYYYMMDD-HHMM}-{topic}` via `scripts/leap.sh agent-start <agent> <topic>`).
- **One Leap packet (`L` packet) per branch:** Implement each `L` packet on its own `leap/L{N}-…` branch (`scripts/leap.sh start leap/L{N}-…`).
- **Strict merge gate:** Merge `leap/L{N}-…` into `main` only via `scripts/leap.sh finish` once stable and at **100% CTest pass** (`agent/*` branches never merge directly to `main`).

## Completed
- L1–L16: OS 2.0 Master Build Plan (`P0`–`P7`) — Capabilities, Federation, Governance, Desktop Operator, 5-Service Session Ensure, Plan Route, Node Execute, Mission Control Console, Core HA Backup/Restore, NixOS Lab Gates, Multi-Workspace Fleet Scale & Replace Drill
- L17–L27: JARVIS Completion Pathway (`WP-00`–`WP-13`) — Observe After Act (`goal-execute`), Policy-Bounded Curiosity Probe (`curiosity-probe`), Unified Inventory Allowlist (`inventory-sync`, `app-check`), Canonical Purpose Library (`purpose-seed`, `purpose-inspect`), Desktop A11y/Window Drive (`desktop-drive`), Voice Router (`voice-route`), Hardening Doctor (`doctor --fix`) + Unified Mission Control UI, NixOS ISO/VM Lab Build & Serial Boot Verification, Default `chat-tools` Router (`56797b1`)
- L28: Zero-warning C++23 build (`apps/cli/main.cpp`), persistent `systemd --user` `sovereign-caps-ui.service`, and updated `HANDOFF.md` + OS 2.0 roadmap docs (`L28`–`L31`)

## In progress / Starting point
- **Next packet:** `leap/L29-autonomous-multi-turn-tool-loop` (Autonomous multi-turn tool dispatch & synthesis loop in `sovereign-cli chat-tools`), followed by `L30` (live window/A11y & Piper TTS audio) and `L31` (multi-node peer enrollment & physical USB flash / bare-metal boot)
