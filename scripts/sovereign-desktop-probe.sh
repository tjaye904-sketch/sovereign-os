#!/usr/bin/env bash
# Item 5: best-effort window list + screenshot on X11 / XWayland / grim
set -euo pipefail
OUT_DIR="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}/computer_use/artifacts"
mkdir -p "$OUT_DIR"
STAMP=$(date -u +%Y%m%dT%H%M%SZ)
SHOT="$OUT_DIR/desktop_probe_${STAMP}.png"
echo "DISPLAY=${DISPLAY:-} WAYLAND_DISPLAY=${WAYLAND_DISPLAY:-} XDG_SESSION_TYPE=${XDG_SESSION_TYPE:-}"
if command -v wmctrl >/dev/null; then
  echo "=== wmctrl -l ==="
  wmctrl -l || true
elif command -v xdotool >/dev/null; then
  xdotool search --name '' 2>/dev/null | head || true
else
  echo "SOFT: no wmctrl/xdotool"
fi
ok=0
if command -v gnome-screenshot >/dev/null; then
  gnome-screenshot -f "$SHOT" 2>/dev/null && ok=1 || true
fi
if [[ "$ok" -eq 0 ]] && command -v import >/dev/null; then
  import -window root "$SHOT" 2>/dev/null && ok=1 || true
fi
if [[ "$ok" -eq 0 ]] && command -v grim >/dev/null; then
  grim "$SHOT" 2>/dev/null && ok=1 || true
fi
if [[ "$ok" -eq 0 ]] && command -v scrot >/dev/null; then
  scrot "$SHOT" 2>/dev/null && ok=1 || true
fi
if [[ "$ok" -eq 1 && -f "$SHOT" ]]; then
  echo "screenshot=$SHOT"
  ls -la "$SHOT"
  exit 0
fi
echo "SOFT: screenshot unavailable on this session (Cosmic/Wayland limits common)"
exit 0
