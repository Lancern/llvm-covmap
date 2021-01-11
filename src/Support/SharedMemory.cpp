//
// Created by Sirui Mu on 2021/1/11.
//

#include "llvm-covmap/Support/SharedMemory.h"

#include <cerrno>
#include <system_error>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

SharedMemory::SharedMemory(const char *name, size_t size)
  : _name(name),
    _size(size),
    _base(nullptr),
    _fd(0)
{
  _fd = shm_open(name, O_RDWR | O_CREAT, 0666);
  if (_fd == -1) {
    throw std::system_error { std::make_error_code(static_cast<std::errc>(errno)), "shm_open failed" };
  }
  if (ftruncate(_fd, size) == -1) {
    auto errorCode = errno;
    close(_fd);
    shm_unlink(name);
    throw std::system_error { std::make_error_code(static_cast<std::errc>(errorCode)), "ftruncate failed" };
  }

  _base = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);
  if (_base == MAP_FAILED) {
    auto errorCode = errno;
    close(_fd);
    shm_unlink(name);
    throw std::system_error { std::make_error_code(static_cast<std::errc>(errorCode)), "mmap failed" };
  }
}

SharedMemory::~SharedMemory() noexcept {
  if (!_base || _base == MAP_FAILED) {
    return;
  }

  munmap(_base, _size);
  close(_fd);
  shm_unlink(_name);

  _base = nullptr;
}
