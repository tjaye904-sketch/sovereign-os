# Sovereign OS live/installer image module (NixOS)
{ config, pkgs, lib, ... }:

{
  isoImage = {
    isoName = lib.mkForce "sovereign-os-${config.system.nixos.label}-${pkgs.stdenv.hostPlatform.system}.iso";
    volumeID = lib.mkForce "SOVEREIGN_OS";
    makeEfiBootable = true;
    makeUsbBootable = true;
    squashfsCompression = "zstd -Xcompression-level 6";
  };

  # Boot / kernel
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
  # installation-cd-minimal enables wireless; avoid NM conflict
  networking.networkmanager.enable = lib.mkForce false;
  networking.wireless.enable = lib.mkDefault true;
  networking.firewall.allowedTCPPorts = [ 22 ];

  # Primary OS surface
  environment.systemPackages = with pkgs; [
    sovereign-cli
    sovereign-daemon
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
    # argon2 linked into sovereign-cli at build time
  ];

  # Sovereign user (live)
  users.users.sovereign = {
    isNormalUser = true;
    description = "Sovereign OS operator";
    extraGroups = [ "wheel" "networkmanager" "video" "audio" ];
    initialPassword = "sovereign";
  };
  users.users.root.initialPassword = "sovereign";
  security.sudo.wheelNeedsPassword = false;

  # Services: daemon as primary orchestration
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

  # First-login banner
  environment.etc."motd".text = ''
    ============================================
      Sovereign OS (NixOS live image)
    ============================================
      CLI:     sovereign-cli
      Status:  sovereign-cli status
      Inspect: sovereign-cli inspect
      Brief:   sovereign-cli brief
      User:    sovereign / sovereign  (change immediately)
    ============================================
  '';

  services.getty.helpLine = ''
    Sovereign OS live — login: sovereign / sovereign
  '';

  # Observability / SSH for recovery
  services.openssh.enable = true;
  services.openssh.settings.PermitRootLogin = "yes";

  # Locale
  time.timeZone = "UTC";
  i18n.defaultLocale = "en_US.UTF-8";
  console.keyMap = "us";

  system.stateVersion = "24.11";
}
