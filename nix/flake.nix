{
  description = "Sovereign OS — NixOS live/installer ISO (x86_64)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.11";
    # Repo root (parent of nix/) so C++ sources are in the eval — not ../.. from store
    sovereign-src = {
      url = "path:..";
      flake = false;
    };
  };

  outputs = { self, nixpkgs, sovereign-src }:
    let
      system = "x86_64-linux";
      lib = nixpkgs.lib;
      pkgs = import nixpkgs { inherit system; };
      # Filter bulky / generated paths out of src
      src = builtins.path {
        path = sovereign-src;
        name = "sovereign-src";
        filter = path: type:
          let b = baseNameOf path;
          in b != "build" && b != ".git" && b != "dist"
             && b != "result" && !(lib.hasPrefix "result-" b)
             && b != "node_modules";
      };
      sovOverlay = final: prev: {
        sovereign-cli = prev.callPackage ./packages/sovereign-cli.nix { inherit src; };
        sovereign-daemon = prev.callPackage ./packages/sovereign-daemon.nix { inherit src; };
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

        sovereign-vm-minimal =
          self.nixosConfigurations.sovereign-vm-minimal.config.system.build.vm;

        sovereign-vm-no-overlay =
          self.nixosConfigurations.sovereign-system-bare.config.system.build.vm;

        sovereign-vm =
          self.nixosConfigurations.sovereign-system.config.system.build.vm;
      };

      nixosConfigurations.sovereign-vm-minimal = nixpkgs.lib.nixosSystem {
        inherit system;
        modules = [ ./modules/sovereign-vm-minimal.nix ];
      };

      nixosConfigurations.sovereign-system-bare = nixpkgs.lib.nixosSystem {
        inherit system;
        modules = [ ./modules/sovereign-system.nix ];
      };

      nixosConfigurations.sovereign-system = nixpkgs.lib.nixosSystem {
        inherit system;
        modules = [
          ./modules/sovereign-system.nix
          { nixpkgs.overlays = [ sovOverlay ]; }
        ];
      };

      nixosConfigurations.sovereign-iso = nixpkgs.lib.nixosSystem {
        inherit system;
        modules = [
          "${nixpkgs}/nixos/modules/installer/cd-dvd/installation-cd-minimal.nix"
          ./modules/sovereign-iso.nix
          { nixpkgs.overlays = [ sovOverlay ]; }
        ];
      };
    };
}
