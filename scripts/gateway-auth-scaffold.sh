#!/usr/bin/env bash
# Gap 10: optional bearer token file for future gateway; off by default
set -euo pipefail
DIR="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}/gateway"
mkdir -p "$DIR"
TOKEN_FILE="$DIR/auth_token"
if [[ ! -f "$TOKEN_FILE" ]]; then
  head -c 32 /dev/urandom | xxd -p -c 64 > "$TOKEN_FILE"
  chmod 600 "$TOKEN_FILE"
  echo "[gateway] created $TOKEN_FILE"
else
  echo "[gateway] token exists $TOKEN_FILE"
fi
# Document env for future C++/Python gateway
ENVF="$HOME/.config/sovereign/env"
grep -q SOVEREIGN_GATEWAY_AUTH_REQUIRED "$ENVF" 2>/dev/null || \
  echo "SOVEREIGN_GATEWAY_AUTH_REQUIRED=0" >> "$ENVF"
grep -q SOVEREIGN_GATEWAY_TOKEN_FILE "$ENVF" 2>/dev/null || \
  echo "SOVEREIGN_GATEWAY_TOKEN_FILE=$TOKEN_FILE" >> "$ENVF"
echo "[gateway] SOVEREIGN_GATEWAY_AUTH_REQUIRED=0 (enable=1 when gateway enforces token)"
cat << DOC
# Operator note
# When API gateway is bound beyond localhost, set:
#   SOVEREIGN_GATEWAY_AUTH_REQUIRED=1
# Clients send: Authorization: Bearer \$(cat \$SOVEREIGN_GATEWAY_TOKEN_FILE)
DOC
