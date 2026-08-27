# Installed / VM Sovereign OS configuration
# Keep systemd ExecStart identical in form to the working live-ISO module.
{ config, pkgs, lib, ... }:

let
  firstBoot = pkgs.writeShellScriptBin "sovereign-first-boot" ''
    set -euo pipefail
    export SOVEREIGN_DATA_ROOT="''${SOVEREIGN_DATA_ROOT:-/var/lib/sovereign}"
    mkdir -p "$SOVEREIGN_DATA_ROOT"
    MARK="$SOVEREIGN_DATA_ROOT/.first-boot-complete"
    if [ -f "$MARK" ]; then
      echo "[first-boot] already complete"
      exit 0
    fi
    echo "=== Sovereign first boot ==="
    echo "data_root=$SOVEREIGN_DATA_ROOT"
    if [ -n "''${SOVEREIGN_BOOTSTRAP_OWNER:-}" ] && [ -n "''${SOVEREIGN_BOOTSTRAP_SECRET:-}" ]; then
      sovereign-cli bootstrap "$SOVEREIGN_BOOTSTRAP_OWNER" "$SOVEREIGN_BOOTSTRAP_SECRET" || true
      sovereign-cli login "$SOVEREIGN_BOOTSTRAP_OWNER" "$SOVEREIGN_BOOTSTRAP_SECRET" || true
      date -Is > "$MARK"
      echo "[first-boot] auto bootstrap complete"
      exit 0
    fi
    echo "Run: sovereign-cli bootstrap <name> <secret> && touch $MARK"
  '';
in
{
  networking.hostName = lib.mkDefault "sovereign";

  environment.systemPackages = with pkgs; [
    sovereign-cli
    sovereign-daemon
    firstBoot
    git
    vim
    htop
    tmux
    curl
  ];

  environment.variables.SOVEREIGN_DATA_ROOT = "/var/lib/sovereign";

  users.users.sovereign = {
    isNormalUser = true;
    extraGroups = [ "wheel" ];
    initialPassword = "sovereign";
  };
  security.sudo.wheelNeedsPassword = false;

  systemd.tmpfiles.rules = [
    "d /var/lib/sovereign 0750 sovereign sovereign -"
  ];

  # Match live-ISO form (known good for ISO build).
  # Args after the binary are fine as a single systemd ExecStart string.
  systemd.services.sovereign-daemon = {
    description = "Sovereign OS agent orchestrator";
    wantedBy = [ "multi-user.target" ];
    after = [ "network-online.target" ];
    wants = [ "network-online.target" ];
    serviceConfig = {
      Type = "simple";
      User = "sovereign";
      Environment = [
        "SOVEREIGN_DATA_ROOT=/var/lib/sovereign"
      ];
      ExecStart = "${pkgs.sovereign-daemon}/bin/sovereign-daemon 60";
      Restart = "on-failure";
      RestartSec = "5";
      StateDirectory = "sovereign";
    };
  };

  # Optional oneshot: only run auto-bootstrap when env is set (same as ISO guidance)
  systemd.services.sovereign-first-boot = {
    description = "Sovereign first-boot identity helper";
    wantedBy = [ "multi-user.target" ];
    after = [ "local-fs.target" ];
    before = [ "sovereign-daemon.service" ];
    serviceConfig = {
      Type = "oneshot";
      RemainAfterExit = true;
      User = "sovereign";
      Environment = [ "SOVEREIGN_DATA_ROOT=/var/lib/sovereign" ];
      ExecStart = "${firstBoot}/bin/sovereign-first-boot";
    };
  };

  services.openssh.enable = true;
  services.getty.autologinUser = lib.mkDefault "sovereign";
  documentation.enable = lib.mkDefault false;
  system.stateVersion = "24.11";
}
