# Installed (non-live) Sovereign OS configuration — Phase XVIII path
{ config, pkgs, lib, ... }:
{
  networking.hostName = lib.mkDefault "sovereign";
  environment.systemPackages = with pkgs; [
    sovereign-cli
    sovereign-daemon
    git vim htop tmux curl
  ];
  environment.variables.SOVEREIGN_DATA_ROOT = "/var/lib/sovereign";
  users.users.sovereign = {
    isNormalUser = true;
    extraGroups = [ "wheel" ];
    initialPassword = "sovereign";
  };
  security.sudo.wheelNeedsPassword = false;
  systemd.tmpfiles.rules = [ "d /var/lib/sovereign 0750 sovereign sovereign -" ];
  systemd.services.sovereign-daemon = {
    description = "Sovereign OS agent orchestrator";
    wantedBy = [ "multi-user.target" ];
    after = [ "network-online.target" ];
    serviceConfig = {
      Type = "simple";
      User = "sovereign";
      Environment = [ "SOVEREIGN_DATA_ROOT=/var/lib/sovereign" ];
      ExecStart = "${pkgs.sovereign-daemon}/bin/sovereign-daemon 60";
      Restart = "on-failure";
      StateDirectory = "sovereign";
    };
  };
  services.openssh.enable = true;
  system.stateVersion = "24.11";
}
