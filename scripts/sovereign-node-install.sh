#!/usr/bin/env bash
set -euo pipefail
TARGET="${1:?target required (user@host or local:/path)}"
NODE_NAME="${2:?node-name required}"
CLI="$(command -v "${SOVEREIGN_CLI:-$HOME/.local/bin/sovereign-cli}")"
DAEMON="${SOVEREIGN_DAEMON:-$(dirname "$CLI")/sovereign-daemon}"
STAGE="$(mktemp -d /tmp/sov-install.XXXXXX)"
trap 'rm -rf "$STAGE"' EXIT
"$CLI" bootstrap owner sovereign-core >/dev/null 2>&1 || true
BUNDLE="$("$CLI" peer-export "$STAGE/trust-bundle" | sed -n 's/^bundle=//p')"
if [[ "$TARGET" == local:* ]]; then
  DEST="${TARGET#local:}"
  mkdir -p "$DEST/bin" "$DEST/data"
  cp -a "$BUNDLE" "$DEST/sovereign-bundle"
  install -m 0755 "$CLI" "$DEST/bin/sovereign-cli"
  [[ -x "$DAEMON" ]] && install -m 0755 "$DAEMON" "$DEST/bin/sovereign-daemon" || true
  SOVEREIGN_DATA_ROOT="$DEST/data" "$DEST/bin/sovereign-cli" bootstrap worker sovereign-worker >/dev/null 2>&1 || true
  SOVEREIGN_DATA_ROOT="$DEST/data" "$DEST/bin/sovereign-cli" peer-import "$DEST/sovereign-bundle" >/dev/null
  ENDPOINT="local://$DEST"
else
  ssh -o BatchMode=yes -o ConnectTimeout=5 "$TARGET" "mkdir -p ~/.local/bin ~/.local/share/sovereign"
  scp -q -r "$BUNDLE" "$CLI" "$TARGET:~/.local/share/sovereign/"
  ssh -o BatchMode=yes "$TARGET" "install -m 0755 ~/.local/share/sovereign/sovereign-cli ~/.local/bin/sovereign-cli && ~/.local/bin/sovereign-cli peer-import ~/.local/share/sovereign/$(basename "$BUNDLE") >/dev/null"
  ENDPOINT="$TARGET:8765"
fi
"$CLI" node-register-peer "$NODE_NAME" "$ENDPOINT" >/dev/null || true
"$CLI" node-heartbeat "$NODE_NAME" >/dev/null || true
echo "NODE_PROVISIONED name=$NODE_NAME endpoint=$ENDPOINT authority=enrolled_compute_only"
