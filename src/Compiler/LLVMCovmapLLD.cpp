//
// Created by Sirui Mu on 2021/1/10.
//

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

#include <unistd.h>

#include "Configure.h"

static const char *GetLLDPath() noexcept {
  auto path = getenv("LLVM_COVMAP_LLD");
  if (path) {
    return path;
  }

  return "ld.lld";
}

int main(int argc, char **argv) {
  std::vector<const char *> args;

  args.push_back(GetLLDPath());

  args.push_back("--library-path=" LLVM_COVMAP_BINARY_DIR "/lib");

  for (auto i = 1; i < argc; ++i) {
    args.push_back(argv[i]);
  }

  args.push_back("--library=LLVMCovmap");

  execvp(args[0], const_cast<char *const *>(args.data()));

  auto errorCode = errno;
  std::cerr << "llvm-covmap-lld: execvp failed: " << errorCode << ": " << strerror(errorCode) << std::endl;

  return 1;
}
