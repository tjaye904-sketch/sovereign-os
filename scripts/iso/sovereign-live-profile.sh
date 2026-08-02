#!/bin/bash
# Installed into live user profile on ISO
export SOVEREIGN_DATA_ROOT="${SOVEREIGN_DATA_ROOT:-$HOME/.local/share/sovereign}"
export PATH="/usr/local/bin:$PATH"
if [[ -z "${SOVEREIGN_WELCOMED:-}" ]]; then
  export SOVEREIGN_WELCOMED=1
  echo "============================================"
  echo "  Sovereign OS — cognitive operating system"
  echo "============================================"
  echo "  CLI:    sovereign-cli"
  echo "  Status: sovereign-cli status"
  echo "  Inspect: sovereign-cli inspect"
  echo "  Brief:  sovereign-cli brief"
  echo "  Help:   sovereign-ctl help"
  echo "============================================"
fi
