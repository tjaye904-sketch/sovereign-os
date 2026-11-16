#!/usr/bin/env bash
# Item 4: keep repo purposes/, data_root/purposes, and CU purposes in sync
set -euo pipefail
ROOT="${1:-$HOME/src/sovereign-os/sovereign-work}"
DATA="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}"
mkdir -p "$DATA/purposes" "$ROOT/purposes"
# Seed defaults
for id in research.web_collect ops.desktop_probe; do
  f="$ROOT/purposes/${id}.yaml"
  if [[ ! -f "$f" ]]; then
    case "$id" in
      research.web_collect)
        cat > "$f" << 'YAML'
purpose_id: research.web_collect
description: Open allowlisted docs URL, snapshot, screenshot
risk: low
require_headed: true
allowed_hosts: [docs.python.org, docs.github.com, en.wikipedia.org, manpages.ubuntu.com, github.com]
allowed_tools: [browser_open, browser_snapshot, browser_screenshot]
steps:
  - {action: browser_open, target: "{url}"}
  - {action: browser_snapshot}
  - {action: browser_screenshot}
YAML
        ;;
      ops.desktop_probe)
        cat > "$f" << 'YAML'
purpose_id: ops.desktop_probe
description: List windows and desktop screenshot
risk: low
require_headed: true
allowed_tools: [desktop_list_windows, desktop_screenshot]
steps:
  - {action: desktop_list_windows}
  - {action: desktop_screenshot}
YAML
        ;;
    esac
  fi
done
cp -a "$ROOT/purposes/"*.yaml "$DATA/purposes/" 2>/dev/null || true
# CU often reads from package or data root
if [[ -d "$HOME/.local/share/sovereign-computer-use" ]]; then
  mkdir -p "$HOME/.local/share/sovereign-computer-use/purposes"
  cp -a "$DATA/purposes/"*.yaml "$HOME/.local/share/sovereign-computer-use/purposes/" 2>/dev/null || true
fi
echo "[sync-purposes] $DATA/purposes:"
ls -la "$DATA/purposes"
