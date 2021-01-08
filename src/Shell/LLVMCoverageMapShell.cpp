//
// Created by Sirui Mu on 2021/1/7.
//

#include <cassert>
#include <cerrno>
#include <climits>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cxxopts.hpp>

namespace {

struct SharedMemory {
  int fd;
  void *mem;
};

__attribute__((noreturn))
void FatalError(const char *function, int errorCode) {
  fprintf(stderr, "%s failed: %d: %s", function, errorCode, strerror(errorCode));
  abort();
}

int StartChild(const std::vector<std::string> &args, const std::string &shmemName, size_t shmemSize) noexcept {
  std::vector<std::string> env;
  for (auto e = environ; *e; ++e) {
    env.emplace_back(*e);
  }
  env.push_back(std::string("LLVM_COVMAP_SHM_NAME=") + shmemName);
  env.push_back(std::string("LLVM_COVMAP_SHM_SIZE=") + std::to_string(shmemSize));

  auto argsNative = std::make_unique<char *[]>(args.size() + 1);
  for (size_t i = 0; i < args.size(); ++i) {
    argsNative[i] = const_cast<char *>(args[i].data());
  }
  argsNative[args.size()] = nullptr;

  auto envNative = std::make_unique<char *[]>(env.size() + 1);
  for (size_t i = 0; i < env.size(); ++i) {
    envNative[i] = const_cast<char *>(env[i].data());
  }
  envNative[env.size()] = nullptr;

  execvpe(argsNative[0], argsNative.get(), envNative.get());

  FatalError("execvpe", errno);
}

SharedMemory MountSharedMemory(const std::string &shmemName, size_t shmemSize) noexcept {
  auto shmemFd = shm_open(shmemName.data(), O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  if (shmemFd == -1) {
    FatalError("shm_open", errno);
  }

  if (ftruncate64(shmemFd, shmemSize) == -1) {
    FatalError("ftruncate64", errno);
  }

  auto shmem = mmap(nullptr, shmemSize, PROT_READ | PROT_WRITE, MAP_SHARED, shmemFd, 0);
  if (shmem == MAP_FAILED) {
    FatalError("mmap", errno);
  }

  return {
    .fd = shmemFd,
    .mem = shmem,
  };
}

void UnmountSharedMemory(const std::string &shmemName, SharedMemory shmem, size_t shmemSize) noexcept {
  munmap(shmem.mem, shmemSize);
  close(shmem.fd);
  shm_unlink(shmemName.data());
}

void DumpCoverageInfo(void *coverageBitmap, size_t shmemSize) noexcept {
  assert((reinterpret_cast<uintptr_t>(coverageBitmap) & 7) && "coverageBitmap is not properly aligned");
  assert((shmemSize & 7) && "shmemSize is not a multiple of 8");

  size_t totalBits = shmemSize * CHAR_BIT;
  size_t effectiveBits = 0;

  auto ptr = reinterpret_cast<uint64_t *>(coverageBitmap);
  while (shmemSize) {
    effectiveBits += __builtin_popcountll(*ptr++);
    shmemSize -= 8;
  }

  auto ratio = static_cast<double>(effectiveBits) / totalBits;
  std::cout << "Coverage "
      << effectiveBits << " / " << totalBits
      << " (" << ratio * 100 << ")"
      << std::endl;
}

int StartParent(pid_t pid, const std::string &shmemName, size_t shmemSize) noexcept {
  auto shmem = MountSharedMemory(shmemName, shmemSize);

  int status;
  do {
    auto ret = waitpid(pid, &status, 0);
    if (ret == -1) {
      FatalError("waitpid", errno);
    }
  } while (!WIFEXITED(status) && !WIFSIGNALED(status));

  if (WIFEXITED(status)) {
    auto exitCode = WEXITSTATUS(status);
    std::cout << "Program exited normally, exit code = " << exitCode << std::endl;
  } else if (WIFSIGNALED(status)) {
    auto sig = WTERMSIG(status);
    std::cout << "Program killed by signal, signal is " << sig << std::endl;
  }

  DumpCoverageInfo(shmem.mem, shmemSize);
  UnmountSharedMemory(shmemName, shmem, shmemSize);

  return 0;
}

} // namespace <anonymous>

int main(int argc, char *argv[]) {
  cxxopts::Options options { "llvm-covmap-shell", "Execute " };
  options.add_options()
      ("h,help", "Dump help message")
      ("p,name", "The name of the shared bitmap memory",
          cxxopts::value<std::string>()
              ->default_value("LLVMCovmap"))
      ("s,size", "Size of the coverage bitmap, in bytes",
          cxxopts::value<size_t>()
              ->default_value("1048576"))
      ("args", "The arguments to the program to be run",
          cxxopts::value<std::vector<std::string>>());
  options.parse_positional("args");

  auto args = options.parse(argc, argv);
  if (args.count("help")) {
    std::cout << options.help() << std::endl;
    return 0;
  }

  if (args["args"].as<std::vector<std::string>>().empty()) {
    std::cerr << "No program to run" << std::endl;
    return 1;
  }

  auto &programArgs = args["args"].as<std::vector<std::string>>();
  auto &name = args["name"].as<std::string>();
  auto size = args["size"].as<size_t>();

  if (size & 7) {
    std::cerr << "Coverage bitmap size should be a multiple of 8" << std::endl;
    return 1;
  }

  auto pid = fork();
  if (pid == 0) {
    return StartChild(programArgs, name, size);
  } else {
    return StartParent(pid, name, size);
  }
}
