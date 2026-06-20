#!/usr/bin/env bash
nix-shell --run "gdb -q -ex 'set confirm off' -ex 'run' -ex 'bt' -ex 'quit' ./build/ftdd" << 'GDB_INPUT'
^[[B
^[[B

q
GDB_INPUT
