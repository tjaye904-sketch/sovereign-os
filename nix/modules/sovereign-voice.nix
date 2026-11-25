{ config, pkgs, lib, ... }:
let
  piperModel = pkgs.fetchurl {
    url = "https://huggingface.co";
    sha256 = "5efe09e69902187827af646e1a6e9d269dee769f9877d17b16b1b46eeaaf019f";
  };
  piperConfig = pkgs.fetchurl {
    url = "https://huggingface.co.json";
    sha256 = "efe19c417bed055f2d69908248c6ba650fa135bc868b0e6abb3da181dab690a0";
  };
  whisperModel = pkgs.fetchurl {
    url = "https://huggingface.co";
    sha256 = "sha256-88ZEgynBnRmY4QFLZGRAxIcKtbQmJtK/PFTI75Ve/Rw=";
  };
  hubChat = pkgs.writeShellScriptBin "sovereign-hub-chat" ''
    set -euo pipefail
    export SOVEREIGN_DATA_ROOT="''${SOVEREIGN_DATA_ROOT:-/var/lib/sovereign}"
    exec sovereign-cli chat-tools "$@"
  '';
  voiceTalk = pkgs.writeShellScriptBin "sovereign-voice-talk" ''
    set -uo pipefail
    export SOVEREIGN_DATA_ROOT="''${SOVEREIGN_DATA_ROOT:-/var/lib/sovereign}"
    LISTEN_SECS="''${SOVEREIGN_LISTEN_SECS:-5}"
    TRANSCRIPT=""
    if [[ "''${1:-}" == "--text" && -n "''${2:-}" ]]; then
      TRANSCRIPT="$2"
    else
      TMP_DIR="$(mktemp -d /tmp/sov-voice.XXXXXX)"
      trap 'rm -rf "$TMP_DIR"' EXIT
      WAV="$TMP_DIR/input.wav"
      echo "[sovereign-voice-talk] Listening for ''${LISTEN_SECS}s on microphone..."
      arecord -q -f S16_LE -r 16000 -c 1 -d "$LISTEN_SECS" "$WAV"
      echo "[sovereign-voice-talk] Transcribing locally with Whisper..."
      whisper-cli -m /etc/sovereign/voices/ggml-base.en.bin -f "$WAV" -nt 2>/dev/null | tr '\n' ' ' | sed 's/^[[:space:]]*//;s/[[:space:]]*$//' > "$TMP_DIR/out.txt"
      TRANSCRIPT="$(cat "$TMP_DIR/out.txt")"
    fi
    [[ -n "$TRANSCRIPT" ]] || { echo "[sovereign-voice-talk] No speech detected."; exit 0; }
    echo "heard: $TRANSCRIPT"
    sovereign-cli voice-route --json listen "$TRANSCRIPT" >/dev/null 2>&1 || true
    RAW_OUT="$(sovereign-hub-chat "$TRANSCRIPT" 2>&1)"
    REPLY="$(printf '%s\n' "$RAW_OUT" | sed -n 's/^content=//p' | tail -n 1 | sed 's/\\n/ /g')"
    [[ -n "$REPLY" ]] || REPLY="$(printf '%s\n' "$RAW_OUT" | grep -vE '^(tool_call |model=|tool_calls_executed=)' | tail -n 1)"
    [[ -n "$REPLY" ]] || REPLY="Sovereign processed your request."
    echo "reply: $REPLY"
    sovereign-cli voice-route --json speak "$REPLY" >/dev/null 2>&1 || true
    echo "VOICE_TALK_TURN_OK"
  '';
in
{
  hardware.enableRedistributableFirmware = true;
  security.rtkit.enable = true;
  services.pipewire = {
    enable = true;
    alsa.enable = true;
    pulse.enable = true;
  };
  environment.systemPackages = with pkgs; [
    alsa-utils
    pipewire
    piper-tts
    openai-whisper-cpp
    ollama
    hubChat
    voiceTalk
  ];
  environment.etc."sovereign/voices/en_US-lessac-medium.onnx".source = piperModel;
  environment.etc."sovereign/voices/en_US-lessac-medium.onnx.json".source = piperConfig;
  environment.etc."sovereign/voices/ggml-base.en.bin".source = whisperModel;
  environment.interactiveShellInit = ''
    alias v='sovereign-voice-talk'
  '';
}
