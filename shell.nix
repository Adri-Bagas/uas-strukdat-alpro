{ pkgs ? import <nixpkgs> {} }:

let
  llvm = pkgs.llvmPackages_latest;
in
pkgs.mkShell {
  packages = with pkgs; [
    cmake
    pkg-config
    vlc
    ncurses
    llvm.clang
    llvm.clang-tools
    
    # Required for FetchContent
    git
    cacert
    
    watchexec

    nlohmann_json
  ];

  shellHook = ''
    export CC=clang
    export CXX=clang++
  '';
}