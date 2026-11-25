{
  description = "Sovereign OS — product tree + NixOS ISO/VM";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.11";
  };

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      lib = nixpkgs.lib;
      pkgs = import nixpkgs { inherit system; };
      src = builtins.path {
        path = self;
        name = "sovereign-src";
        filter = path: type:
          let b = baseNameOf path;
          in b != "build" && b != ".git" && b != "dist"
             && b != "result" && !(lib.hasPrefix "result-" b)
             && b != "node_modules";
      };
      sovOverlay = final: prev: {
        sovereign-cli = prev.callPackage ./nix/packages/sovereign-cli.nix { inherit src; };
        sovereign-daemon = prev.callPackage ./nix/packages/sovereign-daemon.nix { inherit src; };
      };
    in
    {
      packages.${system} = {
        default = self.packages.${system}.sovereign-cli;

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

        sovereign-node-vm =
          self.nixosConfigurations.sovereign-node.config.system.build.vm;
      };

      nixosConfigurations.sovereign-vm-minimal = nixpkgs.lib.nixosSystem {
        inherit system;
        modules = [ ./nix/modules/sovereign-vm-minimal.nix ];
      };

      nixosConfigurations.sovereign-system-bare = nixpkgs.lib.nixosSystem {
        inherit system;
        modules = [ ./nix/modules/sovereign-system.nix ];
      };

      nixosConfigurations.sovereign-node = nixpkgs.lib.nixosSystem {
        inherit system;
        modules = [
          ./nix/modules/sovereign-node.nix
          { nixpkgs.overlays = [ sovOverlay ]; }
        ];
      };

      nixosConfigurations.sovereign-system = nixpkgs.lib.nixosSystem {
        inherit system;
        modules = [
          ./nix/modules/sovereign-system.nix
          { nixpkgs.overlays = [ sovOverlay ]; }
        ];
      };

      checks.${system} = {
        iso-eval = self.packages.${system}.iso;
        vm-eval = self.packages.${system}.sovereign-vm;
        node-vm-eval = self.packages.${system}.sovereign-node-vm;
      };

      nixosConfigurations.sovereign-iso = nixpkgs.lib.nixosSystem {
        inherit system;
        modules = [
          "${nixpkgs}/nixos/modules/installer/cd-dvd/installation-cd-minimal.nix"
          ./nix/modules/sovereign-iso.nix
          ./nix/modules/sovereign-voice.nix
          { nixpkgs.overlays = [ sovOverlay ]; }
        ];
      };
    };
}
