# Sovereign OS live/installer image module (NixOS)
{ config, pkgs, lib, ... }:

let
  firstBoot = pkgs.writeShellScriptBin "sovereign-first-boot" ''
    set -euo pipefail
    export SOVEREIGN_DATA_ROOT="''${SOVEREIGN_DATA_ROOT:-/var/lib/sovereign}"
    if ! mkdir -p "$SOVEREIGN_DATA_ROOT" 2>/dev/null; then sudo mkdir -p "$SOVEREIGN_DATA_ROOT" && sudo chown -R sovereign:users "$SOVEREIGN_DATA_ROOT"; fi
    if [ ! -d "$SOVEREIGN_DATA_ROOT/identity" ]; then
      echo "=== Sovereign first boot ==="
      echo "No identity store. Bootstrap an owner:"
      echo "  sovereign-cli bootstrap <name> <secret>"
      echo "  sovereign-cli login <name> <secret>"
      echo "  sovereign-cli brief"
    else
      echo "Sovereign data at $SOVEREIGN_DATA_ROOT"
      sovereign-cli status 2>/dev/null || true
    fi
  '';
in
{
  isoImage = {
    isoName = lib.mkForce "sovereign-os-${config.system.nixos.label}-${pkgs.stdenv.hostPlatform.system}.iso";
    volumeID = lib.mkForce "SOVEREIGN_OS";
    makeEfiBootable = true;
    makeUsbBootable = true;
    squashfsCompression = "zstd -Xcompression-level 6";
  };

  boot.kernelParams = [
    "console=ttyS0,115200n8"
    "console=tty0"
    "sovereign.mode=normal"
  ];
  boot.supportedFilesystems = [ "btrfs" "ext4" "vfat" "ntfs" "xfs" ];
  boot.initrd.availableKernelModules = [
    "ahci" "xhci_pci" "virtio_pci" "virtio_blk" "virtio_net"
    "usb_storage" "sd_mod" "sr_mod" "nvme" "kvm"
  ];

  networking.hostName = "sovereign";
  networking.networkmanager.enable = lib.mkForce false;
  networking.wireless.enable = lib.mkDefault true;
  networking.firewall.allowedTCPPorts = [ 22 ];

  environment.systemPackages = with pkgs; [
    sovereign-cli
    sovereign-daemon
    firstBoot
    git
    vim
    htop
    tmux
    curl
    wget
    pciutils
    usbutils
    lm_sensors
    openssl
  ];

  environment.variables.SOVEREIGN_DATA_ROOT = "/var/lib/sovereign";
  environment.variables.SOVEREIGN_PROFILE = "nixos-sovereign";
  environment.etc."sovereign-release".text = "SOVEREIGN_OS=2.0\nSUBSTRATE=nixos\nMILESTONE=WP-13\n";
  environment.variables.OLLAMA_HOST = "http://127.0.0.1:11434";
  environment.variables.SOVEREIGN_OLLAMA_MODEL = "openclaw";
  environment.variables.SOVEREIGN_EMBED_MODEL = "nomic-embed-text";

  users.users.sovereign = {
    isNormalUser = true;
    description = "Sovereign OS operator";
    extraGroups = [ "wheel" "video" "audio" ];
    initialPassword = "sovereign";
  };
  users.users.root.initialPassword = "sovereign";
  security.sudo.wheelNeedsPassword = false;

  services.getty.autologinUser = lib.mkForce "sovereign";
  users.users.nixos.initialPassword = lib.mkForce "sovereign";

  systemd.tmpfiles.rules = [
    "d /var/lib/sovereign 0750 sovereign sovereign -"
  ];

  systemd.services.sovereign-daemon = {
    description = "Sovereign OS continuous agent loop";
    wantedBy = [ "multi-user.target" ];
    after = [ "network-online.target" ];
    wants = [ "network-online.target" ];
    serviceConfig = {
      Type = "simple";
      User = "sovereign";
      Environment = [
        "SOVEREIGN_DATA_ROOT=/var/lib/sovereign"
        "OLLAMA_HOST=http://127.0.0.1:11434"
        "SOVEREIGN_OLLAMA_MODEL=openclaw"
        "SOVEREIGN_EMBED_MODEL=nomic-embed-text"
      ];
      ExecStart = "${pkgs.sovereign-daemon}/bin/sovereign-daemon 60";
      Restart = "on-failure";
      RestartSec = "5";
      StateDirectory = "sovereign";
    };
  };

  environment.etc."motd".text = ''
    ============================================
      Sovereign OS (NixOS live image)
    ============================================
      CLI:       sovereign-cli
      Status:    sovereign-cli status
      Inspect:   sovereign-cli inspect
      First boot: sovereign-first-boot
      Bootstrap: sovereign-cli bootstrap owner 'secret'
      User:      sovereign / sovereign  (change immediately)
    ============================================
  '';

  services.getty.helpLine = lib.mkForce ''
    Sovereign OS live — autologin: sovereign / sovereign
  '';

  environment.interactiveShellInit = ''
    if [ -z "''${SOVEREIGN_FIRST_BOOT_SHOWN:-}" ]; then
      export SOVEREIGN_FIRST_BOOT_SHOWN=1
      command -v sovereign-first-boot >/dev/null && sovereign-first-boot || true
    fi
  '';

  services.openssh.enable = true;
  services.openssh.settings.PermitRootLogin = "yes";

  time.timeZone = "UTC";
  i18n.defaultLocale = "en_US.UTF-8";
  console.keyMap = "us";

  system.stateVersion = "24.11";
}
