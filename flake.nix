{
  inputs = {
    utils.url = "github:numtide/flake-utils";
  };
  outputs = {
    self,
    nixpkgs,
    utils,
  }:
    utils.lib.eachDefaultSystem (
      system: let
        pkgs = nixpkgs.legacyPackages.${system};
        rEnv = pkgs.rWrapper.override {
          packages = with pkgs.rPackages; [
            here
            tidyverse
            janitor
            DoE_base
          ];
        };
      in {
        devShell = pkgs.mkShell {
          buildInputs = with pkgs; [
            openmpi
            rEnv
          ];
        };
      }
    );
}
