//
// Created by Sirui Mu on 2021/1/3.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-reserved-identifier"

#include <cassert>
#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

constexpr static const size_t DefaultSharedMemorySize = 1024 * 1024;  // 1 MiB

static bool disabled;
static int sharedMemoryFd;
static std::mutex mountMutex;

extern "C" {
  uint8_t *__llvm_covmap;
  const char *__llvm_covmap_shm_name;
  size_t __llvm_covmap_size;
} // extern "C"

__attribute__((noreturn))
static void FatalError(const char *function, int errorCode) noexcept {
  fprintf(stderr, "llvm-covmap: %s failed: %d: %s\n", function, errorCode, strerror(errorCode));
  abort();
}

static size_t GetSharedMemorySize() noexcept {
  auto sharedMemorySizeStr = getenv("LLVM_COVMAP_SHM_SIZE");
  if (!sharedMemorySizeStr) {
    return DefaultSharedMemorySize;
  }

  errno = 0;
  size_t sharedMemorySize = std::strtoull(sharedMemorySizeStr, nullptr, 10);
  if (errno != 0) {
    return DefaultSharedMemorySize;
  }

  assert(((sharedMemorySize & 7) == 0) && "Shared memory size should be a multiple of 8");
  return sharedMemorySize;
}

static void UnmountBitmap() noexcept {
  if (disabled || !__llvm_covmap) {
    return;
  }

  munmap(__llvm_covmap, __llvm_covmap_size);
  close(sharedMemoryFd);
  shm_unlink(__llvm_covmap_shm_name);
}

static void MountBitmap() noexcept {
  __llvm_covmap_shm_name = getenv("LLVM_COVMAP_SHM_NAME");
  if (!__llvm_covmap_shm_name) {
    disabled = true;
    return;
  }

  sharedMemoryFd = shm_open(__llvm_covmap_shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  if (sharedMemoryFd == -1) {
    FatalError("shm_open", errno);
  }

  __llvm_covmap_size = GetSharedMemorySize();
  if (ftruncate64(sharedMemoryFd, __llvm_covmap_size) == -1) {
    auto errorCode = errno;
    close(sharedMemoryFd);
    shm_unlink(__llvm_covmap_shm_name);
    FatalError("ftruncate64", errorCode);
  }

  auto sharedMemory = mmap(nullptr, __llvm_covmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, sharedMemoryFd, 0);
  if (sharedMemory == MAP_FAILED) {
    auto errorCode = errno;
    close(sharedMemoryFd);
    shm_unlink(__llvm_covmap_shm_name);
    FatalError("mmap", errorCode);
  }

  __llvm_covmap = reinterpret_cast<uint8_t *>(sharedMemory);

  atexit(UnmountBitmap);
}

__attribute__((always_inline))
static inline void SetBitmap(uint64_t functionId) noexcept {
  auto offset = functionId % (__llvm_covmap_size * CHAR_BIT);
  __llvm_covmap[offset >> 3] |= (1u << (offset & 7));
}

extern "C" {

  void __llvm_covmap_hit_function(uint64_t functionId) {
    if (disabled) {
      return;
    }

    if (!__llvm_covmap) {
      mountMutex.lock();

      if (disabled) {
        return;
      }

      if (!__llvm_covmap) {
        MountBitmap();
        if (!__llvm_covmap) {
          disabled = true;
        }
      }
      mountMutex.unlock();

      if (disabled) {
        return;
      }
    }

    SetBitmap(functionId);
  }

} // extern "C"

#pragma clang diagnostic pop