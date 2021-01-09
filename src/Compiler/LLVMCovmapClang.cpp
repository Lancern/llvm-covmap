//
// Created by Sirui Mu on 2021/1/9.
//

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <unistd.h>

#include "Configure.h"

#ifdef LLVM_COVMAP_CLANG_PLUSPLUS
constexpr static const char *ClangName = "clang++";
#else
constexpr static const char *ClangName = "clang";
#endif

constexpr static const char *PassModule = LLVM_COVMAP_BINARY_DIR "/lib/libLLVMCoverageMapPass.so";

#define LLVM_COVMAP_RUNTIME_LIBRARY_DIR \
  LLVM_COVMAP_BINARY_DIR "/lib"
#define LLVM_COVMAP_RUNTIME_LIBRARY_NAME "LLVMCovmap"

static const char* GetClangPath() noexcept {
  auto clangPath = getenv("LLVM_COVMAP_CLANG");
  if (clangPath) {
    return clangPath;
  }

  return ClangName;
}

static void ExecuteClang(const std::vector<std::string> &args) noexcept {
  if (getenv("LLVM_COVMAP_CLANG_DEBUG")) {
    std::cerr << "llvm-covmap-clang: debug: executing clang with args: [" << std::endl;
    for (const auto &e : args) {
      std::cerr << "    \"" << e << "\"," << std::endl;
    }
    std::cerr << "]" << std::endl;
  }

  std::vector<const char *> nativeArgs;
  nativeArgs.reserve(args.size() + 1);

  for (const auto &e : args) {
    nativeArgs.push_back(e.data());
  }
  nativeArgs.push_back(nullptr);

  // According to POSIX standards, exec series of functions change neither the pointer arrays nor the strings referenced
  // by the pointer arrays. So the const_cast in the following code should be safe.
  execvp(nativeArgs[0], const_cast<char *const *>(nativeArgs.data()));

  auto errorCode = errno;
  auto errorMessage = strerror(errorCode);
  std::cerr << "llvm-covmap-clang: execvp failed: " << errorCode << ": " << errorMessage << std::endl;
}

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

  args.emplace_back(GetClangPath());
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

  ExecuteClang(args);
  return 1;
}
