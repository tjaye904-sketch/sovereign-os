{
  description = "Sovereign OS — NixOS live/installer ISO (x86_64)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.11";
  };

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      lib = nixpkgs.lib;
      pkgs = import nixpkgs { inherit system; };
      sovOverlay = final: prev: {
        sovereign-cli = final.callPackage ./packages/sovereign-cli.nix { };
        sovereign-daemon = final.callPackage ./packages/sovereign-daemon.nix { };
      };
    in
    {
      packages.${system} = {
        iso = self.nixosConfigurations.sovereign-iso.config.system.build.isoImage;

        sovereign-cli = (import nixpkgs {
          inherit system;
          overlays = [ sovOverlay ];
        }).sovereign-cli;

        sovereign-daemon = (import nixpkgs {
          inherit system;
          overlays = [ sovOverlay ];
        }).sovereign-daemon;

        # Minimal VM (no Sovereign pkgs) — proves system.build.vm works
        sovereign-vm-minimal =
          self.nixosConfigurations.sovereign-vm-minimal.config.system.build.vm;

        # Full system VM (packages on PATH; start daemon manually)
        sovereign-vm =
          self.nixosConfigurations.sovereign-system.config.system.build.vm;
      };

      nixosConfigurations.sovereign-vm-minimal = nixpkgs.lib.nixosSystem {
        inherit system;
        modules = [ ./modules/sovereign-vm-minimal.nix ];
      };

      nixosConfigurations.sovereign-system = nixpkgs.lib.nixosSystem {
        inherit system;
        modules = [
          ./modules/sovereign-system.nix
          {
            nixpkgs.overlays = [ sovOverlay ];
            boot.loader.grub.device = "nodev";
            boot.loader.grub.enable = true;
            fileSystems."/" = {
              device = "none";
              fsType = "tmpfs";
              options = [ "defaults" "mode=755" ];
            };
          }
        ];
      };

      nixosConfigurations.sovereign-iso = nixpkgs.lib.nixosSystem {
        inherit system;
        modules = [
          "${nixpkgs}/nixos/modules/installer/cd-dvd/installation-cd-minimal.nix"
          ./modules/sovereign-iso.nix
          {
            nixpkgs.overlays = [ sovOverlay ];
          }
        ];
      };
    };
}
