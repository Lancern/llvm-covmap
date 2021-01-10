//
// Created by Sirui Mu on 2021/1/10.
//

#include <iostream>
#include <string>
#include <vector>

#include "Configure.h"
#include "Utils.hpp"

constexpr const char *LLDName = "ld.lld";
constexpr const char *LLDPathEnvName = "LLVM_COVMAP_LLD";
constexpr const char *WrapperName = "llvm-covmap-lld";

int main(int argc, char **argv) {
  std::vector<std::string> args;

  args.push_back(LookupUtilityPath(LLDName, LLDPathEnvName));

  args.emplace_back("--library-path=" LLVM_COVMAP_BINARY_DIR "/lib");

  for (auto i = 1; i < argc; ++i) {
    args.emplace_back(argv[i]);
  }

  args.emplace_back("--library=LLVMCovmap");

  ExecuteUtility(args, WrapperName, "lld");
  return 1;
}
