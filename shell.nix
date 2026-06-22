{ pkgs ? import <nixpkgs> {} }:

let
  llvm = pkgs.llvmPackages_latest;
in
pkgs.mkShell {
  packages = with pkgs;
  [
    cmake
    pkg-config
    ncurses
    llvm.clang
    llvm.clang-tools
    
    # Driver Audio
    alsa-lib
    libpulseaudio
    
    # Required for FetchContent
    git
    cacert
    
    watchexec
    gdb

    nlohmann_json
  ];
  
  shellHook = ''
    export CC=clang
    export CXX=clang++
    export LD_LIBRARY_PATH="${pkgs.libpulseaudio}/lib:${pkgs.alsa-lib}/lib:$LD_LIBRARY_PATH"
    
    # KUNCI UTAMA ANTI PUTUS-PUTUS DI PERTENGAHAN LAGU (WSL BUG FIX)
    export PULSE_LATENCY_MSEC=200
  '';
}