#!/usr/bin/env bash
set -euo pipefail

export PATH="$HOME/.local/bin:$HOME/.local/share/sovereign-voice/bin:$PATH"
if [[ -f "$HOME/.config/sovereign/env" ]]; then
  set -a
  # shellcheck disable=SC1091
  source "$HOME/.config/sovereign/env"
  set +a
fi

CLI="${SOVEREIGN_CLI:-$HOME/.local/bin/sovereign-cli}"
CHAT_BIN="${SOVEREIGN_HUB_CHAT:-$HOME/.local/bin/sovereign-hub-chat}"
WHISPER_BIN="${SOVEREIGN_WHISPER_BIN:-$HOME/.local/share/sovereign-voice/bin/whisper}"
LISTEN_SECS="${SOVEREIGN_LISTEN_SECS:-5}"
WHISPER_MODEL="${SOVEREIGN_WHISPER_MODEL:-base.en}"

TRANSCRIPT=""
if [[ "${1:-}" == "--text" && -n "${2:-}" ]]; then
  TRANSCRIPT="$2"
else
  TMP_DIR="$(mktemp -d /tmp/sov-voice.XXXXXX)"
  trap 'rm -rf "$TMP_DIR"' EXIT
  WAV="$TMP_DIR/input.wav"
  echo "[sovereign-voice-talk] Listening for ${LISTEN_SECS}s on Dell microphone..." >&2
  if command -v arecord >/dev/null 2>&1; then
    arecord -q -f S16_LE -r 16000 -c 1 -d "$LISTEN_SECS" "$WAV"
  elif command -v pw-record >/dev/null 2>&1; then
    timeout "${LISTEN_SECS}s" pw-record --rate 16000 --channels 1 "$WAV" || true
  else
    echo "ERROR: neither arecord nor pw-record found" >&2
    exit 1
  fi
  echo "[sovereign-voice-talk] Transcribing locally with Whisper ($WHISPER_MODEL)..." >&2
  "$WHISPER_BIN" "$WAV" --model "$WHISPER_MODEL" --language en --output_format txt --output_dir "$TMP_DIR" >/dev/null 2>&1
  TRANSCRIPT="$(tr '\n' ' ' < "$TMP_DIR/input.txt" 2>/dev/null | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')"
fi

if [[ -z "$TRANSCRIPT" ]]; then
  echo "[sovereign-voice-talk] No speech detected." >&2
  exit 0
fi

echo "heard: $TRANSCRIPT"
"$CLI" voice-route --json listen "$TRANSCRIPT" >/dev/null
"$CLI" voice-route --json intent "$TRANSCRIPT" >/dev/null || true

RAW_OUT="$("$CHAT_BIN" "$TRANSCRIPT")"
REPLY="$(printf '%s\n' "$RAW_OUT" | sed -n 's/^content=//p' | tail -n 1)"
[[ -n "$REPLY" ]] || REPLY="$RAW_OUT"
echo "reply: $REPLY"

"$CLI" voice-route --json speak "$REPLY" >/dev/null || true
echo "VOICE_TALK_TURN_OK"
