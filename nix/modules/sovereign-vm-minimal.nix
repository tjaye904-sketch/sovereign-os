# Bare NixOS config for system.build.vm smoke — no Sovereign packages.
{ lib, ... }:
{
  networking.hostName = "sovereign-vm";
  boot.loader.grub.device = "nodev";
  boot.loader.grub.enable = true;
  fileSystems."/" = {
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
  services.getty.autologinUser = "sovereign";
  services.openssh.enable = false;
  documentation.enable = false;
  system.stateVersion = "24.11";
}
