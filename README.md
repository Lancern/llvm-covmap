# llvm-covmap

![Build Passing](https://img.shields.io/badge/build-passing-brightgree)
![LLVM 10.0.0](https://img.shields.io/badge/llvm-10.0.0-blue)

LLVM passes and utilities that instruments code to profile for **function-level** 
code coverage via bitmap during program runtime.

## Build

### Build Prerequisites

- CMake version >= 3.12
- LLVM

To install LLVM on your platform, execute the following command:

```shell
sudo apt install llvm-dev
```

It's recommended to use LLVM version 10.0.0. Other versions of LLVM might work
as well, but they are not tested. To check the LLVM version on your system, execute
the following command:

```shell
llvm-config --version
```

### Build Steps

Clone the repository:

```shell
git clone https://github.com/Lancern/llvm-covmap.git
cd llvm-covmap
```

Create a build directory:

```shell
mkdir build
cd build
```

Build the project using the familiar two-step build:

```shell
cmake ..
cmake --build .
```

The output files are generated in the `lib/` directory in the build tree.

## Usage

> For advanced usage of `llvm-covmap`, please refer to [docs](./docs).

To simplify examples, we assume that the `LLVM_COVMAP_BUILD_DIR` environment is set
to the root of the build tree.

Instrument LLVM modules by hand:

```shell
opt -load $LLVM_COVMAP_BUILD_DIR/lib/libLLVMCoverageMapPass.so -covmap \
  -o=instrumented-module.bc \
  input-module.bc
```

End-to-end build using the drop-in replacement of `clang` and `clang++`:

```shell
# Compile only, do not link
$LLVM_COVMAP_BUILD_DIR/bin/llvm-covmap-clang -c -o example.o example.c

# End-to-end compile is supported
# All necessary runtime libraries required by llvm-covmap will be linked automatically
$LLVM_COVMAP_BUILD_DIR/bin/llvm-covmap-clang -o example example.c
$LLVM_COVMAP_BUILD_DIR/bin/llvm-covmap-clang++ -o example example.cpp
```

One-shot run an **instrumented** program and dump the coverage information:

```shell
$LLVM_COVMAP_BUILD_DIR/bin/llvm-covmap-shell ls
```

## Contribute

This project is built for private use so we don't accept any feature requests or
pull requests. But you're free to fork your own copy of this project and add
modifications to fit your need.

## License

This repository is open-sourced under the [MIT license](./LICENSE).
