//
// Created by Sirui Mu on 2021/1/9.
//

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <unistd.h>

#ifdef LLVM_COVMAP_CLANG_PLUSPLUS
constexpr static const char *ClangName = "clang++";
#else
constexpr static const char *ClangName = "clang";
#endif

constexpr static const char *SourceFileExtensions[4] = {
    "c",
    "cc",
    "cpp",
    "cxx",
};

static const char* GetClangPath() noexcept {
  auto clangPath = getenv("LLVM_COVMAP_CLANG");
  if (clangPath) {
    return clangPath;
  }

  return ClangName;
}

static bool IsSourceFileName(const char *arg) noexcept {
  auto extStart = strrchr(arg, '.');
  if (!extStart) {
    return false;
  }

  ++extStart;
  return std::any_of(
      std::begin(SourceFileExtensions), std::end(SourceFileExtensions),
      [extStart] (const char *ext) noexcept -> bool {
        return strcmp(ext, extStart) == 0;
      });
}

static bool IsLinking(int argc, char **argv) noexcept {
  auto sourceFileCount = 0;
  for (auto i = 1; i < argc; ++i) {
    if (IsSourceFileName(argv[i])) {
      ++sourceFileCount;
    }
  }

  return sourceFileCount != 1;
}

static void ExecuteClang(int argc, char **argv, const std::vector<std::string> &additionalArgs) noexcept {
  std::vector<const char *> args;
  args.reserve(argc + additionalArgs.size() + 1);

  args.push_back(GetClangPath());
  for (auto i = 1; i < argc; ++i) {
    args.push_back(argv[i]);
  }

  for (const auto &aa : additionalArgs) {
    args.push_back(aa.data());
  }

  args.push_back(nullptr);

  // According to POSIX standards, exec series of functions change neither the pointer arrays nor the strings referenced
  // by the pointer arrays. So the const_cast in the following code should be safe.
  execvp(args[0], const_cast<char *const *>(args.data()));

  auto errorCode = errno;
  auto errorMessage = strerror(errorCode);
  std::cerr << argv[0] << ": execvp failed: " << errorCode << ": " << errorMessage << std::endl;
}

int main(int argc, char **argv) {
  std::vector<std::string> additionalArgs;
  if (IsLinking(argc, argv)) {
    additionalArgs.reserve(2);
    
  }

  ExecuteClang(argc, argv, additionalArgs);
  return 1;
}
