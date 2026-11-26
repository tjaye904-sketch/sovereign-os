#!/usr/bin/env bash
set -euo pipefail
TARGET="${1:?target required}"
WORKSPACE="${2:?workspace required}"
EXTRA_APP="${3:-}"
case "$WORKSPACE" in
  security)       PKGS="nmap masscan wireshark tcpdump aircrack-ng john hashcat sqlmap nikto gobuster metasploit ghidra radare2" ;;
  intelligence)   PKGS="ollama whisper-cpp piper-tts nvtop python3 jq curl" ;;
  development)    PKGS="git cmake ninja gcc rustc cargo python3 gdb valgrind ripgrep" ;;
  robotics)       PKGS="python3 usbutils can-utils minicom i2c-tools gpsd" ;;
  creative)       PKGS="blender gimp inkscape ffmpeg imagemagick krita" ;;
  media)          PKGS="ffmpeg vlc audacity sox pipewire" ;;
  research)       PKGS="python3 pandoc jq curl ripgrep sqlite" ;;
  operations)     PKGS="htop btop sysstat tmux rsync openssh iproute2" ;;
  automation)     PKGS="jq yq curl python3 sqlite tmux" ;;
  finance)        PKGS="gnupg sqlite python3 qrencode libreoffice" ;;
  governance)     PKGS="sqlite gnupg jq git" ;;
  communications) PKGS="openssh wireguard-tools curl openssl mosh" ;;
  home)           PKGS="avahi mosquitto iproute2 pipewire" ;;
  education)      PKGS="pandoc python3 sqlite okular" ;;
  health)         PKGS="python3 sqlite jq gnupg" ;;
  personal)       PKGS="sqlite tar zstd gnupg tmux" ;;
  *) echo "unknown workspace pack: $WORKSPACE" >&2; exit 1 ;;
esac
STORE_AND_TERM="repoman flatpak tmux konsole"
ALL_PKGS="$PKGS $STORE_AND_TERM${EXTRA_APP:+ $EXTRA_APP}"
if [[ "$TARGET" == local:* ]]; then
  DEST="${TARGET#local:}"
  mkdir -p "$DEST"
  printf '{"workspace":"%s","store":"repoman+flatpak","terminal":"tmux+konsole","packages":"%s","extra_app":"%s"}\n' "$WORKSPACE" "$PKGS" "$EXTRA_APP" > "$DEST/pack-manifest.json"
  [[ -n "$EXTRA_APP" ]] && echo "$EXTRA_APP" >> "$DEST/custom-apps.list"
else
  ssh -o BatchMode=yes -o ConnectTimeout=5 "$TARGET" "if command -v nix-env >/dev/null 2>&1; then nix-env -iA nixpkgs.flatpak $ALL_PKGS 2>/dev/null || true; elif command -v apt-get >/dev/null 2>&1; then sudo apt-get update -qq && sudo apt-get install -y -qq $ALL_PKGS; fi"
fi
echo "PACK_INSTALLED target=$TARGET workspace=$WORKSPACE store=repoman terminal=tmux+konsole"
