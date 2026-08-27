# Installed / VM Sovereign OS configuration
# NOTE: Do NOT declare systemd ExecStart with package interpolations here for
# system.build.vm — NixOS vmVariant validates ExecStart as store paths and
# fails with `path '/' is not a valid store path` when args/overlays interact
# poorly. Live ISO keeps its own daemon unit in sovereign-iso.nix (works).
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

  # Daemon: enable via path unit later; for now start manually in VM:
  #   sovereign-daemon 60
  # Live ISO module still auto-starts the daemon.

  services.openssh.enable = true;
  services.getty.autologinUser = lib.mkDefault "sovereign";
  documentation.enable = lib.mkDefault false;
  system.stateVersion = "24.11";
}
