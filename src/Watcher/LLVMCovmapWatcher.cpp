//
// Created by Sirui Mu on 2021/1/11.
//

#include <cerrno>
#include <chrono>
#include <climits>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <iostream>
#include <memory>
#include <string>
#include <system_error>

#include <cxxopts.hpp>

#include "llvm-covmap/Support/SharedMemory.h"

static volatile bool Interrupted;
static void (*PreviousInterruptHandler)(int);

struct CoverageRecord {
  uint64_t timestamp;
  uint64_t covered;
  uint64_t total;
  double ratio;
};

void InterruptHandler(int sig) noexcept {
  if (sig != SIGINT) {
    return;
  }
  Interrupted = true;
  if (PreviousInterruptHandler) {
    PreviousInterruptHandler(sig);
  }
}

void CountCoverage(const SharedMemory &shm, CoverageRecord &record) noexcept {
  record.timestamp = std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::seconds(1);
  record.covered = 0;
  record.total = shm.size() * CHAR_BIT;

  auto ptr = reinterpret_cast<const uint64_t *>(shm.base());
  auto size = shm.size();

  while (size > 0) {
    record.covered += __builtin_popcountll(*ptr++);
    size -= 8;
  }

  record.ratio = static_cast<double>(record.covered) / record.total;
}

void WatcherLoop(const SharedMemory &shm, unsigned interval) noexcept {
  std::cout << "time,covered,total,ratio" << std::endl;

  timespec intervalTime = {
      .tv_sec = interval,
      .tv_nsec = 0,
  };
  timespec intervalRemains = intervalTime;

  CoverageRecord coverage; // NOLINT(cppcoreguidelines-pro-type-member-init)

  while (!Interrupted) {
    if (nanosleep(&intervalRemains, &intervalRemains) == -1) {
      auto errorCode = errno;
      if (errorCode != EINTR) {
        std::cerr << "nanosleep failed: " << errorCode << ": " << strerror(errorCode) << std::endl;
        return;
      }
      if (Interrupted) {
        break;
      }
      continue;
    }

    CountCoverage(shm, coverage);
    std::cout << coverage.timestamp << ","
        << coverage.covered << ","
        << coverage.total << ","
        << coverage.ratio << std::endl;

    intervalRemains = intervalTime;
  }
}

int main(int argc, char **argv) {
  cxxopts::Options options {
    "llvm-covmap-watcher",
    "Watch for coverage bitmap and dump real-time coverage to disk files"
  };

  options.add_options()
      ("h,help", "Dump help message")
      ("p,name", "The name of the shared bitmap memory",
          cxxopts::value<std::string>()
              ->default_value("LLVMCovmap"))
      ("s,size", "Size of the coverage bitmap, in bytes",
          cxxopts::value<size_t>()
              ->default_value("1048576"))
      ("t,interval", "The interval between two consecutive coverage samplings, in seconds",
          cxxopts::value<unsigned>()
              ->default_value("10"));

  auto args = options.parse(argc, argv);

  const auto& shmName = args["name"].as<std::string>();
  auto shmSize = args["size"].as<size_t>();
  auto interval = args["interval"].as<unsigned>();

  if (shmSize & 7) {
    std::cerr << "The size of the shared memory should be a multiple of 8" << std::endl;
    return 1;
  }

  std::unique_ptr<SharedMemory> shm;
  try {
    shm = std::make_unique<SharedMemory>(shmName.c_str(), shmSize);
  } catch (const std::system_error &err) {
    std::cerr << "Cannot open shared memory: " << err.what() << std::endl;
    return 1;
  }

  PreviousInterruptHandler = signal(SIGINT, InterruptHandler);
  if (PreviousInterruptHandler == SIG_ERR) {
    std::cerr << "signal failed" << std::endl;
    return 1;
  }

  WatcherLoop(*shm, interval);

  return 0;
}
