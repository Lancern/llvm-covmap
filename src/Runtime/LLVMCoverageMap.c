//
// Created by Sirui Mu on 2021/1/3.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-reserved-identifier"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define DEFAULT_SHARED_MEMORY_SIZE (1024 * 1024)

static pthread_mutex_t mountMutex = PTHREAD_MUTEX_INITIALIZER;

const char *__llvm_covmap_shm_name;
int __llvm_covmap_disabled;
int __llvm_covmap_fd;
uint8_t *__llvm_covmap;
size_t __llvm_covmap_size;

__attribute__((noreturn))
static void FatalError(const char *function, int errorCode) {
  __llvm_covmap_disabled = 1;
  fprintf(stderr, "llvm-covmap: %s failed: %d: %s\n", function, errorCode, strerror(errorCode));
  abort();
}

static size_t GetSharedMemorySize() {
  const char *sharedMemorySizeStr = getenv("LLVM_COVMAP_SHM_SIZE");
  if (!sharedMemorySizeStr) {
    return DEFAULT_SHARED_MEMORY_SIZE;
  }

  errno = 0;
  size_t sharedMemorySize = strtoul(sharedMemorySizeStr, NULL, 10);
  if (errno != 0) {
    return DEFAULT_SHARED_MEMORY_SIZE;
  }

  assert(((sharedMemorySize & 7) == 0) && "Shared memory size should be a multiple of 8");
  return sharedMemorySize;
}

static void UnlinkSharedMemory() {
  if (__llvm_covmap_disabled || !__llvm_covmap) {
    return;
  }

  // Let the kernel to unmap the shared memory pages and close the shared memory file descriptor when the process
  // invokes the _exit system call. Otherwise, other atexit handlers might trigger segmentation faults when accessing
  // the bitmap.

  shm_unlink(__llvm_covmap_shm_name);
}

static void MountBitmap() {
  __llvm_covmap_shm_name = getenv("LLVM_COVMAP_SHM_NAME");
  if (!__llvm_covmap_shm_name) {
    __llvm_covmap_disabled = 1;
    return;
  }

  __llvm_covmap_fd = shm_open(__llvm_covmap_shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  if (__llvm_covmap_fd == -1) {
    FatalError("shm_open", errno);
  }

  __llvm_covmap_size = GetSharedMemorySize();
  if (ftruncate(__llvm_covmap_fd, __llvm_covmap_size) == -1) {
    int errorCode = errno;
    close(__llvm_covmap_fd);
    shm_unlink(__llvm_covmap_shm_name);
    FatalError("ftruncate64", errorCode);
  }

  void* sharedMemory = mmap(NULL, __llvm_covmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, __llvm_covmap_fd, 0);
  if (sharedMemory == MAP_FAILED) {
    int errorCode = errno;
    close(__llvm_covmap_fd);
    shm_unlink(__llvm_covmap_shm_name);
    FatalError("mmap", errorCode);
  }

  __llvm_covmap = (uint8_t *)sharedMemory;

  atexit(UnlinkSharedMemory);
}

__attribute__((always_inline))
static inline void SetBitmap(uint64_t functionId) {
  uint64_t offset = functionId % (__llvm_covmap_size * CHAR_BIT);
  __llvm_covmap[offset >> 3] |= (1u << (offset & 7));
}

void __llvm_covmap_hit_function(uint64_t functionId) {
  if (__llvm_covmap_disabled) {
    return;
  }

  if (!__llvm_covmap) {
    if (pthread_mutex_lock(&mountMutex)) {
      FatalError("pthread_mutex_lock", errno);
    }

    if (__llvm_covmap_disabled) {
      if (pthread_mutex_unlock(&mountMutex)) {
        FatalError("pthread_mutex_unlock", errno);
      }
      return;
    }

    if (!__llvm_covmap) {
      MountBitmap();
    }
    if (pthread_mutex_unlock(&mountMutex)) {
      FatalError("pthread_mutex_unlock", errno);
    }

    if (__llvm_covmap_disabled) {
      return;
    }
  }

  SetBitmap(functionId);
}

#pragma clang diagnostic pop