//
// Created by Sirui Mu on 2021/1/3.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-reserved-identifier"

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define SET_BITMAP(map, offset) \
  map[(offset) >> 3] |= (1u << ((offset) & 7))

static bool disabled;
static int sharedMemoryFd;
static std::mutex mountMutex;

extern "C" {
  uint8_t *__llvm_covmap;
} // extern "C"

__attribute__((noreturn))
static void FatalError(const char *function, int errorCode) {
  fprintf(stderr, "%s failed: %d: %s", function, errorCode, strerror(errorCode));
  abort();
}

static uint64_t GetSharedMemorySize(uint64_t bitmapSize) noexcept {
  return (bitmapSize >> 3) + static_cast<uint64_t>((bitmapSize & 7) != 0) + sizeof(uint64_t);
}

static void UnmountBitmap() noexcept {
  auto sharedMemoryName = getenv("LLVM_COVMAP_SHM_NAME");
  if (!sharedMemoryName) {
    return;
  }

  if (!__llvm_covmap) {
    return;
  }

  uint8_t *sharedMemory = __llvm_covmap - sizeof(uint64_t);
  auto sharedMemorySize = GetSharedMemorySize(*reinterpret_cast<uint64_t *>(sharedMemory));

  munmap(sharedMemory, sharedMemorySize);
  close(sharedMemoryFd);
  shm_unlink(sharedMemoryName);
}

static void MountBitmap(uint64_t bitmapSize) noexcept {
  auto sharedMemoryName = getenv("LLVM_COVMAP_SHM_NAME");
  if (!sharedMemoryName) {
    disabled = true;
    return;
  }

  sharedMemoryFd = shm_open(sharedMemoryName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  if (sharedMemoryFd == -1) {
    FatalError("shm_open", errno);
  }

  auto sharedMemorySize = GetSharedMemorySize(bitmapSize);
  if (ftruncate64(sharedMemoryFd, sharedMemorySize) == -1) {
    auto errorCode = errno;
    close(sharedMemoryFd);
    shm_unlink(sharedMemoryName);
    FatalError("ftruncate64", errorCode);
  }

  auto sharedMemory = mmap(nullptr, sharedMemorySize, PROT_READ | PROT_WRITE, MAP_SHARED, sharedMemoryFd, 0);
  if (sharedMemory == MAP_FAILED) {
    auto errorCode = errno;
    close(sharedMemoryFd);
    shm_unlink(sharedMemoryName);
    FatalError("mmap", errorCode);
  }

  *reinterpret_cast<uint64_t *>(sharedMemory) = bitmapSize;
  __llvm_covmap = reinterpret_cast<uint8_t *>(sharedMemory) + sizeof(uint64_t);

  atexit(UnmountBitmap);
}

extern "C" {

  void __llvm_covmap_hit_function(uint64_t numFunctions, uint64_t functionId) {
    if (disabled) {
      return;
    }

    if (!__llvm_covmap) {
      mountMutex.lock();

      if (disabled) {
        return;
      }

      if (!__llvm_covmap) {
        MountBitmap(numFunctions);
        if (!__llvm_covmap) {
          disabled = true;
        }
      }
      mountMutex.unlock();

      if (disabled) {
        return;
      }
    }

    SET_BITMAP(__llvm_covmap, functionId);
  }

} // extern "C"

#pragma clang diagnostic pop