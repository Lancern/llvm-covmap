//
// Created by Sirui Mu on 2021/1/10.
//

#ifndef LLVM_COVMAP_SRC_COMPILER_UTILS_HPP
#define LLVM_COVMAP_SRC_COMPILER_UTILS_HPP

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "unistd.h"

static inline std::string LookupUtilityPath(const char *utilityName, const char *utilityPathEnvName) noexcept {
  auto path = getenv("LLVM_COVMAP_CLANG_BASE_DIR");
  if (path) {
    std::string p(path);
    p.append("/");
    p.append(utilityName);
    return p;
  }

  path = getenv(utilityPathEnvName);
  if (path) {
    return std::string(path);
  }

  return std::string(utilityName);
}

static inline void ExecuteUtility(
    const std::vector<std::string> &args,
    const char *wrapperName,
    const char *utilityName) noexcept {
  if (getenv("LLVM_COVMAP_CLANG_DEBUG")) {
    std::cerr << wrapperName << ": debug: executing " << utilityName << " with args: [" << std::endl;
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
  std::cerr << wrapperName << ": execvp failed: " << errorCode << ": " << errorMessage << std::endl;
}

#endif // LLVM_COVMAP_SRC_COMPILER_UTILS_HPP
