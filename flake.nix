{
  inputs = { utils.url = "github:numtide/flake-utils"; };
  outputs = { self, nixpkgs, utils }:
    utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        mtp = pkgs.stdenv.mkDerivation {
          pname = "mtp";
          version = "0.1.0";
          src = ./.;
          nativeBuildInputs = with pkgs; [ gnumake gcc ];
          buildPhase = "make";
          installPhase = ''
            mkdir -p $out/bin
            cp ./mcts $out/bin/
          '';
        };
      in {
        devShell = pkgs.mkShell {
          buildInputs = with pkgs; [ clang-tools llvmPackages.openmp ];
        };

        apps.default = {
          type = "app";
          program = "${mtp}/bin/mtp 1280 720 50";
        };
      });
}
