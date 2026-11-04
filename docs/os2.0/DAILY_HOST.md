# Sovereign daily driver (Pop!_OS host)

## Services (user systemd)
| Unit | Port | Role |
|------|------|------|
| sovereign-daemon | — | agent tick / orchestrator |
| sovereign-computer-use | 8765 | headed browser / purposes |
| sovereign-sys | 8767 | on-screen apps + sys observe |
| sovereign-hub | 8766 | Mission Control UI |

## Visibility
- `SOVEREIGN_CU_HEADLESS=0` — browser windows visible
- `SOVEREIGN_SYS_HEADLESS=0` — app_open uses real desktop
- Login runs `sovereign-session-start` to import DISPLAY into systemd

## Hub buttons
- **Ensure ready** — start CU/sys/daemon if down
- **Computer-use** — purpose_run, snapshot, screenshot
- **System** — System Monitor, Files, Terminal, VS Code, Firefox, custom allowlisted target
- **Chat** — local Ollama model via sovereign-cli

## Limits
- App allowlist in sovereign-sys policy (extend as needed)
- Full desktop screenshot still weak on some Wayland sessions; page screenshots via CU work
- Not a replacement for NixOS bare-metal control plane yet
