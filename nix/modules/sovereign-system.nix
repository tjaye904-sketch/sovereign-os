# Installed / VM Sovereign OS configuration
{ config, pkgs, lib, ... }:
{
  networking.hostName = lib.mkDefault "sovereign";

  environment.systemPackages = with pkgs; [
    sovereign-cli
    sovereign-daemon
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

  # Daemon: ExecStart must be a single store path for systemd/NixOS coercion.
  # Tick interval via env (daemon accepts optional argv; default 60 in binary).
  systemd.services.sovereign-daemon = {
    description = "Sovereign OS agent orchestrator";
    wantedBy = [ "multi-user.target" ];
    after = [ "network-online.target" "sovereign-first-boot.service" ];
    wants = [ "network-online.target" ];
    serviceConfig = {
      Type = "simple";
      User = "sovereign";
      Environment = [
        "SOVEREIGN_DATA_ROOT=/var/lib/sovereign"
      ];
      ExecStart = lib.getExe pkgs.sovereign-daemon;
      Restart = "on-failure";
      RestartSec = "5s";
      StateDirectory = "sovereign";
    };
  };

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
      ExecStart = lib.getExe (pkgs.writeShellApplication {
        name = "sovereign-first-boot";
        runtimeInputs = [ pkgs.sovereign-cli ];
        text = ''
          set -euo pipefail
          ROOT=/var/lib/sovereign
          mkdir -p "$ROOT"
          MARK="$ROOT/.first-boot-complete"
          if [ -f "$MARK" ]; then
            echo "[first-boot] already complete"
            exit 0
          fi
          echo "=== Sovereign first boot ==="
          echo "data_root=$ROOT"
          if [ -n "''${SOVEREIGN_BOOTSTRAP_OWNER:-}" ] && [ -n "''${SOVEREIGN_BOOTSTRAP_SECRET:-}" ]; then
            sovereign-cli bootstrap "$SOVEREIGN_BOOTSTRAP_OWNER" "$SOVEREIGN_BOOTSTRAP_SECRET" || true
            sovereign-cli login "$SOVEREIGN_BOOTSTRAP_OWNER" "$SOVEREIGN_BOOTSTRAP_SECRET" || true
            date -Is > "$MARK"
            echo "[first-boot] auto bootstrap complete"
            exit 0
          fi
          echo "Run: sovereign-cli bootstrap <name> <secret> && touch $MARK"
          exit 0
        '';
      });
    };
  };

  services.openssh.enable = true;
  services.getty.autologinUser = lib.mkDefault "sovereign";
  documentation.enable = lib.mkDefault false;
  system.stateVersion = "24.11";
}
