{
  inputs = { utils.url = "github:numtide/flake-utils"; };
  outputs = { self, nixpkgs, utils }:
    utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        mcp = pkgs.stdenv.mkDerivation {
          pname = "mcp";
          version = "0.1.0";
          src = ./.;
          nativeBuildInputs = with pkgs; [ gnumake gcc ];
          installPhase = ''
            mkdir -p $out/bin
            cp ./mcts $out/bin/
          '';
        };
        rEnv = pkgs.rWrapper.override {
          packages = with pkgs.rPackages; [
            languageserver
            lintr
            here
            DoE_base
            FrF2
            tidyverse
          ];
        };
      in {
        devShell = pkgs.mkShell {
          buildInputs = [
            pkgs.clang-tools
            pkgs.llvmPackages.openmp
            rEnv
            (pkgs.texlive.combined.scheme-full.withPackages
              (ps: with ps; [ minted ]))
          ];
        };
        packages.default = pkgs.writeShellScriptBin "run-mcp" ''
          ${mcp}/bin/mcts 3840 2160 900
        '';
        apps.default = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/run-mcp";
        };
      });
}
