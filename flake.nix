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
      in {
        devShell = pkgs.mkShell {
          buildInputs = with pkgs; [ clang-tools llvmPackages.openmp ];
        };
        packages.default = pkgs.writeShellScriptBin "run-mcp" ''
          ${mcp}/bin/mcts 1280 720 50
        '';
        apps.default = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/run-mcp";
        };
      });
}
