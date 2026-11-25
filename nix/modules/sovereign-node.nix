# L15 — Headless NixOS workspace node image profile (nixos-sovereign).
{ config, lib, pkgs, ... }:
{
  networking.hostName = lib.mkDefault "sovereign-node";
  boot.loader.grub.device = lib.mkDefault "nodev";
  boot.loader.grub.enable = lib.mkDefault true;

  fileSystems."/" = lib.mkDefault {
    device = "none";
    fsType = "tmpfs";
    options = [ "defaults" "mode=755" ];
  };

  services.xserver.enable = false;
  services.openssh.enable = false;
  networking.firewall.enable = true;
  documentation.enable = false;

  users.groups.sovereign = {};
  users.users.sovereign = {
    isNormalUser = true;
    group = "sovereign";
    home = "/var/lib/sovereign";
    createHome = true;
  };

  environment.systemPackages = with pkgs; [
    sovereign-cli
    sovereign-daemon
  ];

  systemd.services.sovereign-daemon = {
    description = "Sovereign OS 2.0 Headless Workspace Node Daemon";
    wantedBy = [ "multi-user.target" ];
    after = [ "network-online.target" ];
    wants = [ "network-online.target" ];
    environment = {
      SOVEREIGN_DATA_ROOT = "/var/lib/sovereign/data";
      SOVEREIGN_PROFILE = "nixos-sovereign";
    };
    serviceConfig = {
      User = "sovereign";
      Group = "sovereign";
      StateDirectory = "sovereign";
      ExecStart = "${pkgs.sovereign-daemon}/bin/sovereign-daemon 300";
      Restart = "on-failure";
      RestartSec = "5s";
      NoNewPrivileges = true;
      ProtectSystem = "strict";
      ProtectHome = true;
      PrivateTmp = true;
    };
  };

  system.stateVersion = "24.11";
}
