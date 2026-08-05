#!/usr/bin/env bash
# sovereign-ctl — operational control for local Sovereign OS
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="${SOVEREIGN_BIN_DIR:-$ROOT/build}"
CLI="${BIN}/sovereign-cli"
DAEMON="${BIN}/sovereign-daemon"
DATA="${SOVEREIGN_DATA_ROOT:-${HOME}/.local/share/sovereign}"
PIDFILE="${DATA}/daemon.pid"
export SOVEREIGN_DATA_ROOT="$DATA"
export OLLAMA_HOST="${OLLAMA_HOST:-http://127.0.0.1:11434}"
export SOVEREIGN_OLLAMA_MODEL="${SOVEREIGN_OLLAMA_MODEL:-openclaw}"
export SOVEREIGN_EMBED_MODEL="${SOVEREIGN_EMBED_MODEL:-nomic-embed-text}"

cmd="${1:-help}"

build() {
  bash "$ROOT/scripts/build-sovereign-cli.sh" "$CLI"
}

ensure_cli() {
  [[ -x "$CLI" ]] || build
}

case "$cmd" in
  build) build ;;
  status)
    ensure_cli
    "$CLI" status
    if [[ -f "$PIDFILE" ]] && kill -0 "$(cat "$PIDFILE")" 2>/dev/null; then
      echo "daemon=running pid=$(cat "$PIDFILE")"
    else
      echo "daemon=stopped"
    fi
    ;;
  bootstrap)
    ensure_cli
    name="${2:-owner}"; secret="${3:-}"
    if [[ -z "$secret" ]]; then echo "usage: sovereign-ctl bootstrap <name> <secret>"; exit 1; fi
    mkdir -p "$DATA"
    "$CLI" bootstrap "$name" "$secret"
    "$CLI" login "$name" "$secret"
    "$CLI" workspace-init-domains
    "$CLI" agent-add-brief 86400
    "$CLI" goal-add 90 "Operate Sovereign cognitive OS"
    echo "bootstrap_complete data_root=$DATA"
    ;;
  start)
    ensure_cli
    [[ -x "$DAEMON" ]] || build
    mkdir -p "$DATA"
    if [[ -f "$PIDFILE" ]] && kill -0 "$(cat "$PIDFILE")" 2>/dev/null; then
      echo "already running pid=$(cat "$PIDFILE")"; exit 0
    fi
    tick="${2:-60}"
    nohup "$DAEMON" "$tick" >"$DATA/daemon.log" 2>&1 &
    echo $! >"$PIDFILE"
    echo "daemon_started pid=$(cat "$PIDFILE") tick=$tick"
    ;;
  stop)
    if [[ -f "$PIDFILE" ]]; then
      kill "$(cat "$PIDFILE")" 2>/dev/null || true
      rm -f "$PIDFILE"
      echo "daemon_stopped"
    else
      echo "daemon_not_running"
    fi
    ;;
  brief)
    ensure_cli; "$CLI" brief ;;
  tick)
    ensure_cli; "$CLI" agent-tick ;;
  smoke)
    ensure_cli
    bash "$ROOT/scripts/smoke-v1-complete.sh" "$CLI"
    ;;
  help|*)
    cat <<H
sovereign-ctl — local Sovereign OS operations
  build                         compile CLI + daemon
  bootstrap <name> <secret>     first-time owner + domains + daily brief job
  status                        host + daemon status
  start [tick_seconds]          start sovereign-daemon (default 60)
  stop                          stop daemon
  brief                         print system brief
  tick                          run due agent jobs once
  smoke                         offline product smoke
Env: SOVEREIGN_DATA_ROOT, OLLAMA_HOST, SOVEREIGN_OLLAMA_MODEL, SOVEREIGN_EMBED_MODEL
H
    ;;
esac
