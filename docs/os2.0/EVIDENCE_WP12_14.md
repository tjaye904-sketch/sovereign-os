# EVIDENCE WP-12 / WP-13 / WP-14

## WP-12 Desktop reliability
- `desktop_list_windows` via wmctrl → gdbus → xdotool
- `desktop_focus` via wmctrl/xdotool
- `desktop_screenshot` via import → scrot → gnome-screenshot → grim

```bash
sudo apt install -y wmctrl scrot
sovereign-cu desktop_list_windows
sovereign-cu desktop_screenshot --args '{"name":"wp12.png"}'
```

## WP-13 chat-tools → tools
```bash
sovereign-chat-tools "open system monitor"
# or alias: sovereign-cli is unchanged; wrapper sovereign-chat-tools
```

## WP-14 inventory harden
```bash
bash scripts/inventory-harden.sh
sovereign-inventory list | head
# Expect few ALLOWs, rest deny
sovereign-tools --json "open gucharmap"   # deny
```

## Commit
`WP-12/13/14: desktop tools, chat-tools bridge, inventory deny-by-default`
