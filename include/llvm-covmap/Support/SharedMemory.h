//
// Created by Sirui Mu on 2021/1/11.
//

#ifndef LLVM_COVMAP_SUPPORT_SHARED_MEMORY_H
#define LLVM_COVMAP_SUPPORT_SHARED_MEMORY_H

#include <cstddef>

/**
 * A POSIX shared memory object.
 */
class SharedMemory {
public:
  /**
   * Construct a new SharedMemory object.
   *
   * This function throws std::system_error if shared memory creation fails.
   *
   * @param name the name of the shared memory object.
   * @param size the size of the shared memory region, in bytes.
   */
  explicit SharedMemory(const char *name, size_t size);

  SharedMemory(const SharedMemory &) = delete;
  SharedMemory(SharedMemory &&) noexcept = delete;

  SharedMemory& operator=(const SharedMemory &) = delete;
  SharedMemory& operator=(SharedMemory &&) noexcept = delete;

  ~SharedMemory() noexcept;

  /**
   * Get the name of the shared memory object.
   *
   * @return the name of the shared memory object.
   */
  const char* name() const noexcept {
    return _name;
  }

  /**
   * Get the size of the shared memory region, in bytes.
   *
   * @return the size of the shared memory region, in bytes.
   */
  size_t size() const noexcept {
    return _size;
  }

  /**
   * Get a pointer to the first byte within the shared memory region.
   *
   * @return a pointer to the first byte within the shared memory region.
   */
  void* base() const noexcept {
    return _base;
  }

private:
  const char* _name;
  size_t _size;
  void *_base;
  int _fd;
};

#endif // LLVM_COVMAP_SUPPORT_SHARED_MEMORY_H
