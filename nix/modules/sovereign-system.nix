# Full installed/VM config. Built on top of the known-good minimal pattern.
# Sovereign binaries via overlay; no custom systemd ExecStart (breaks vmVariant).
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
  security.sudo.wheelNeedsPassword = false;
  services.getty.autologinUser = lib.mkDefault "sovereign";

  # Keep dbus surface small (openssh pulls dbus; enable when packages OK)
  services.openssh.enable = false;
  documentation.enable = false;

  environment.variables.SOVEREIGN_DATA_ROOT = "/var/lib/sovereign";
  systemd.tmpfiles.rules = [
    "d /var/lib/sovereign 0750 sovereign sovereign -"
  ];

  # Only add Sovereign packages if the overlay defined them
  environment.systemPackages = lib.optionals (pkgs ? sovereign-cli) [
    pkgs.sovereign-cli
    pkgs.sovereign-daemon
  ] ++ (with pkgs; [ git vim curl ]);

  system.stateVersion = "24.11";
}
