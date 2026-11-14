#!/usr/bin/env bash
# WP-19: durable owner secret for daily path (env file, not disk LUKS)
set -euo pipefail
export PATH="${HOME}/.local/bin:${PATH}"
CFG="$HOME/.config/sovereign"
ENVF="$CFG/env"
mkdir -p "$CFG" "${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}"

echo "=== WP-19 session login setup ==="
if [[ -n "${SOVEREIGN_OWNER_SECRET:-}" ]]; then
  SECRET="$SOVEREIGN_OWNER_SECRET"
  echo "using SOVEREIGN_OWNER_SECRET from environment"
else
  read -r -s -p "Owner secret for Sovereign login (input hidden): " SECRET
  echo
  [[ -n "$SECRET" ]] || { echo "empty secret — abort"; exit 1; }
fi

# Merge into env without export keywords (systemd-safe)
touch "$ENVF"
grep -v '^SOVEREIGN_OWNER_SECRET=' "$ENVF" >"$ENVF.tmp" || true
echo "SOVEREIGN_OWNER_SECRET=$SECRET" >>"$ENVF.tmp"
# keep other keys
if ! grep -q '^SOVEREIGN_OWNER_NAME=' "$ENVF.tmp"; then
  echo "SOVEREIGN_OWNER_NAME=owner" >>"$ENVF.tmp"
fi
mv "$ENVF.tmp" "$ENVF"
chmod 600 "$ENVF"

# Bash helper still uses set -a
cat > "$CFG/env.bash" << 'EOF'
set -a
# shellcheck disable=SC1091
. "$HOME/.config/sovereign/env"
set +a
export PATH="$HOME/.local/bin:$PATH"
EOF

# Optional libsecret
if command -v secret-tool >/dev/null; then
  printf '%s' "$SECRET" | secret-tool store --label='Sovereign owner' service sovereign username owner 2>/dev/null || true
  echo "libsecret: attempted store (service=sovereign username=owner)"
fi

# Bootstrap/login against real data root
set -a
# shellcheck disable=SC1091
. "$ENVF"
set +a
export SOVEREIGN_DATA_ROOT="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}"

if ! sovereign-cli login owner "$SECRET" 2>/dev/null; then
  echo "login failed — bootstrap then login"
  sovereign-cli bootstrap owner "$SECRET" || true
  sovereign-cli login owner "$SECRET"
fi

# Patch session chain to prefer env secret
CHAIN="$HOME/src/sovereign-os/sovereign-work/scripts/session-tools-chain.sh"
if [[ -f "$CHAIN" ]]; then
  sed -i 's/login owner "\${SOVEREIGN_OWNER_SECRET:-your-secret}"/login owner "\${SOVEREIGN_OWNER_SECRET}"/' "$CHAIN" 2>/dev/null || true
fi

echo "session_ok"
sovereign-cli status | head -12
echo "WP-19 done — restart shells or: source ~/.config/sovereign/env.bash"
