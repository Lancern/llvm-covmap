#!/usr/bin/env sh

ln -f -s ./llvm-covmap-clang clang
ln -f -s ./llvm-covmap-clang clang-cl

ln -f -s ./llvm-covmap-clang++ clang++

ln -f -s ./llvm-covmap-lld lld
ln -f -s ./llvm-covmap-lld lld-link
ln -f -s ./llvm-covmap-lld ld.lld
ln -f -s ./llvm-covmap-lld ld64.lld

ln -f -s "$(which llvm-ar)" llvm-ar

ln -f -s "$(which llvm-objcopy)" llvm-objcopy

ln -f -s "$(which llvm-pdbutil)" llvm-pdbutil

ln -f -s "$(which llvm-symbolizer)" llvm-symbolizer

ln -f -s "$(which llvm-undname)" llvm-undname
