# llvm-covmap

![Build Passing](https://img.shields.io/badge/build-passing-brightgree)
![LLVM 10.0.0](https://img.shields.io/badge/llvm-10.0.0-blue)

LLVM pass that instruments code to profile for code coverage via bitmap during
program runtime.

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



## Contribute

This project is open for any feature requests and pull requests. Feel free to open
a feature request via the issues zone.

## License

This repository is open-sourced under the [MIT license](./LICENSE).
