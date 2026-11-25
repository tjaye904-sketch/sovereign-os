{ config, pkgs, lib, ... }:
let
  mcLauncher = pkgs.makeDesktopItem {
    name = "sovereign-mission-control";
    desktopName = "Sovereign Mission Control";
    comment = "Open Sovereign OS 2.0 Unified Mission Control UI";
    exec = "${pkgs.firefox}/bin/firefox http://localhost:8769/ui";
    icon = "utilities-system-monitor";
    categories = [ "System" "Monitor" ];
  };
  voiceLauncher = pkgs.makeDesktopItem {
    name = "sovereign-voice-talk";
    desktopName = "Sovereign Voice Talk";
    comment = "Push-to-talk local Whisper STT -> Sovereign Hub -> Piper TTS";
    exec = "${pkgs.kdePackages.konsole}/bin/konsole -e sovereign-voice-talk";
    icon = "audio-input-microphone";
    categories = [ "AudioVideo" "Utility" ];
  };
in
{
  services.xserver.enable = true;
  services.displayManager.sddm.enable = true;
  services.displayManager.sddm.wayland.enable = true;
  services.desktopManager.plasma6.enable = true;

  services.displayManager.autoLogin.enable = true;
  services.displayManager.autoLogin.user = "sovereign";

  # KDE Plasma uses NetworkManager for graphical Wi-Fi & Ethernet
  networking.networkmanager.enable = lib.mkOverride 10 true;
  networking.wireless.enable = lib.mkForce false;

  environment.systemPackages = with pkgs; [
    firefox
    wmctrl
    wl-clipboard
    mcLauncher
    voiceLauncher
  ];
}
