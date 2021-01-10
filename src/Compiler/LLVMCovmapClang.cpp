//
// Created by Sirui Mu on 2021/1/9.
//

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "Configure.h"
#include "Utils.hpp"

#ifdef LLVM_COVMAP_CLANG_PLUSPLUS
constexpr static const char *ClangName = "clang++";
constexpr static const char *ClangPathEnvName = "LLVM_COVMAP_CLANG_PLUSPLUS";
constexpr static const char *WrapperName = "llvm-covmap-clang++";
#else
constexpr static const char *ClangName = "clang";
constexpr static const char *ClangPathEnvName = "LLVM_COVMAP_CLANG";
constexpr static const char *WrapperName = "llvm-covmap-clang";
#endif

constexpr static const char *PassModule = LLVM_COVMAP_BINARY_DIR "/lib/libLLVMCoverageMapPass.so";

#define LLVM_COVMAP_RUNTIME_LIBRARY_DIR \
  LLVM_COVMAP_BINARY_DIR "/lib"
#define LLVM_COVMAP_RUNTIME_LIBRARY_NAME "LLVMCovmap"

static bool IsCompiling(int argc, char **argv) {
  for (auto i = 0; i < argc; ++i) {
    if (strcmp(argv[i], "-c") == 0) {
      return true;
    }
  }

  return false;
}

int main(int argc, char **argv) {
  auto compiling = IsCompiling(argc, argv);

  std::vector<std::string> args;

  args.push_back(LookupUtilityPath(ClangName, ClangPathEnvName));
  args.emplace_back("-Xclang");
  args.emplace_back("-load");
  args.emplace_back("-Xclang");
  args.emplace_back(PassModule);

  if (!compiling) {
    args.emplace_back("-L" LLVM_COVMAP_RUNTIME_LIBRARY_DIR);
  }

  for (auto i = 1; i < argc; ++i) {
    args.emplace_back(argv[i]);
  }

  if (!compiling) {
    args.emplace_back("-l" LLVM_COVMAP_RUNTIME_LIBRARY_NAME);
    args.emplace_back("-lrt");
  }

  ExecuteUtility(args, WrapperName, "clang++");
  return 1;
}
