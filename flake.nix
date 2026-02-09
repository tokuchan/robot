{
  description = "Robot development environment with C++ tools";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            gcc
            gnumake
            cmake
            catch2_3
            boost186
            ccache
            pkg-config
            llvmPackages_18.clang-tools
          ];
          
          shellHook = ''
            # Set up environment for standard CMake find modules
            export CMAKE_PREFIX_PATH="${pkgs.boost186}:${pkgs.catch2_3}:$CMAKE_PREFIX_PATH"
            export LD_LIBRARY_PATH="${pkgs.boost186}/lib:${pkgs.catch2_3}/lib:$LD_LIBRARY_PATH"
            export PKG_CONFIG_PATH="${pkgs.boost186}/lib/pkgconfig:${pkgs.catch2_3}/lib/pkgconfig:$PKG_CONFIG_PATH"
            
            # Help CMake find Boost
            export BOOST_ROOT="${pkgs.boost186}"
            export Boost_DIR="${pkgs.boost186}"
            
            echo "C++ development environment ready!"
            echo "  Boost: ${pkgs.boost186}"
            echo "  Catch2: ${pkgs.catch2_3}"
            echo "  GCC: $(gcc --version | head -n1)"
          '';
        };
      }
    );
}