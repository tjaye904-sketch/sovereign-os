#!/usr/bin/env bash
# Gap 8: observe desktop on X11 + Wayland with multiple backends
set -euo pipefail
OUT="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}/computer_use/artifacts"
mkdir -p "$OUT"
STAMP=$(date -u +%Y%m%dT%H%M%SZ)
SHOT="$OUT/desktop_probe_${STAMP}.png"
echo "session type=${XDG_SESSION_TYPE:-?} DISPLAY=${DISPLAY:-} WAYLAND=${WAYLAND_DISPLAY:-}"

echo "=== windows ==="
if command -v wmctrl >/dev/null 2>&1; then
  wmctrl -l 2>/dev/null || true
fi
if command -v gdbus >/dev/null 2>&1; then
  # GNOME Shell (may fail on Cosmic)
  gdbus call --session --dest org.gnome.Shell --object-path /org/gnome/Shell \
    --method org.gnome.Shell.Eval 'global.get_window_actors().map(a=>a.meta_window.get_title()).filter(Boolean)' \
    2>/dev/null | head -c 2000 || true
fi
command -v hyprctl >/dev/null 2>&1 && hyprctl clients 2>/dev/null | head -40 || true
command -v swaymsg >/dev/null 2>&1 && swaymsg -t get_tree 2>/dev/null | head -c 1500 || true

echo "=== screenshot ==="
ok=0
for try in \
  "gnome-screenshot -f '$SHOT'" \
  "grim '$SHOT'" \
  "import -window root '$SHOT'" \
  "scrot '$SHOT'"; do
  if eval "$try" 2>/dev/null && [[ -f "$SHOT" ]]; then
    ok=1
    break
  fi
done
if [[ "$ok" -eq 1 ]]; then
  echo "screenshot=$SHOT ($(stat -c%s "$SHOT") bytes)"
else
  echo "SOFT: no screenshot tool succeeded"
fi
exit 0
