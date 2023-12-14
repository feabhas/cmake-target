// ReadWriteLock.cpp
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#include "ReadWriteLock.h"

namespace FeabhOS
{
  ReadWriteLock::ReadWriteLock() :
    handle(0)
  {
    feabhOS_rwlock_create(&handle);
  }


  ReadWriteLock::~ReadWriteLock()
  {
    feabhOS_rwlock_destroy(&handle);
  }


  void ReadWriteLock::readAcquire()
  {
    feabhOS_rwlock_read_acquire(&handle);
  }


  void ReadWriteLock::readRelease()
  {
    feabhOS_rwlock_read_release(&handle);
  }


  void ReadWriteLock::writeAcquire()
  {
    feabhOS_rwlock_write_acquire(&handle);
  }


  void ReadWriteLock::writeRelease()
  {
    feabhOS_rwlock_write_release(&handle);
  }

} // namespace FeabhOS


