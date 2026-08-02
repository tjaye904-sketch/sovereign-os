{
  description = "Sovereign OS — NixOS live/installer ISO (x86_64)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.11";
  };

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
    in
    {
      packages.${system} = {
        # Build the live ISO image
        iso = self.nixosConfigurations.sovereign-iso.config.system.build.isoImage;

        # CLI package alone (for host use / tests)
        sovereign-cli = pkgs.callPackage ./packages/sovereign-cli.nix { };
        sovereign-daemon = pkgs.callPackage ./packages/sovereign-daemon.nix { };
      };

      nixosConfigurations.sovereign-system = nixpkgs.lib.nixosSystem {
        inherit system;
        modules = [
          ./modules/sovereign-system.nix
          {
            nixpkgs.overlays = [
              (final: prev: {
                sovereign-cli = final.callPackage ./packages/sovereign-cli.nix { };
                sovereign-daemon = final.callPackage ./packages/sovereign-daemon.nix { };
              })
            ];
            # Minimal boot for installed system evaluation
            boot.loader.grub.device = "nodev";
            fileSystems."/" = { device = "/dev/disk/by-label/nixos"; fsType = "ext4"; };
          }
        ];
      };

      nixosConfigurations.sovereign-iso = nixpkgs.lib.nixosSystem {
        inherit system;
        modules = [
          "${nixpkgs}/nixos/modules/installer/cd-dvd/installation-cd-minimal.nix"
          ./modules/sovereign-iso.nix
          {
            nixpkgs.overlays = [
              (final: prev: {
                sovereign-cli = final.callPackage ./packages/sovereign-cli.nix { };
                sovereign-daemon = final.callPackage ./packages/sovereign-daemon.nix { };
              })
            ];
          }
        ];
      };

      # Convenience apps
      apps.${system}.build-iso = {
        type = "app";
        program = "${pkgs.writeShellScript "build-sovereign-iso" ''
          set -euo pipefail
          echo "Building Sovereign OS ISO via Nix..."
          nix build path:${self}#iso -L
          echo "ISO under: ./result/iso/"
          ls -lh result/iso/ || ls -lh result/
        ''}";
      };
    };
}
