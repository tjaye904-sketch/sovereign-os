# Full installed/VM Sovereign OS configuration
{ config, pkgs, lib, ... }:
{
  networking.hostName = lib.mkDefault "sovereign";

  boot.loader.grub.device = lib.mkDefault "nodev";
  boot.loader.grub.enable = lib.mkDefault true;

  fileSystems."/" = lib.mkDefault {
    device = "none";
    fsType = "tmpfs";
    options = [ "defaults" "mode=755" ];
  };

  users.users.sovereign = {
    isNormalUser = true;
    extraGroups = [ "wheel" ];
    initialPassword = "sovereign";
  };
  # Ensure matching group exists for tmpfiles ownership
  users.groups.sovereign = { };

  security.sudo.wheelNeedsPassword = false;
  services.getty.autologinUser = lib.mkDefault "sovereign";

  # 1) SSH for host copy/paste
  services.openssh.enable = true;
  services.openssh.settings.PermitRootLogin = "yes";
  services.openssh.settings.PasswordAuthentication = true;
  networking.firewall.allowedTCPPorts = [ 22 ];

  documentation.enable = false;

  environment.variables.SOVEREIGN_DATA_ROOT = "/var/lib/sovereign";
  environment.variables.OLLAMA_HOST = "http://10.0.2.2:11434";
  environment.variables.SOVEREIGN_OLLAMA_MODEL = "llama3.1:8b";
  environment.variables.SOVEREIGN_EMBED_MODEL = "nomic-embed-text";

  # 2) Data root owned by sovereign on every boot
  systemd.tmpfiles.rules = [
    "d /var/lib/sovereign 0750 sovereign sovereign -"
    "d /var/lib/sovereign/identity 0750 sovereign sovereign -"
    "d /var/lib/sovereign/audit 0750 sovereign sovereign -"
  ];

  environment.systemPackages = lib.optionals (pkgs ? sovereign-cli) [
    pkgs.sovereign-cli
    pkgs.sovereign-daemon
  ] ++ (with pkgs; [ git vim curl ]);

  # 3) Daemon — ISO-style ExecStart (flake root src=self should allow eval)
  systemd.services.sovereign-daemon = lib.mkIf (pkgs ? sovereign-daemon) {
    description = "Sovereign OS agent orchestrator";
    wantedBy = [ "multi-user.target" ];
    after = [ "network-online.target" "systemd-tmpfiles-setup.service" ];
    wants = [ "network-online.target" ];
    serviceConfig = {
      Type = "simple";
      User = "sovereign";
      Environment = [
        "SOVEREIGN_DATA_ROOT=/var/lib/sovereign"
        "OLLAMA_HOST=http://10.0.2.2:11434"
        "SOVEREIGN_OLLAMA_MODEL=llama3.1:8b"
        "SOVEREIGN_EMBED_MODEL=nomic-embed-text"
      ];
      ExecStart = "${pkgs.sovereign-daemon}/bin/sovereign-daemon 60";
      Restart = "on-failure";
      RestartSec = "5";
      StateDirectory = "sovereign";
    };
  };

  system.stateVersion = "24.11";
}
