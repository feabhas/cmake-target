// ReadWriteLock.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef CPP03_FEABHOS_READWRITELOCK_H
#define CPP03_FEABHOS_READWRITELOCK_H

#include "feabhOS_rwlock.h"

namespace FeabhOS
{
  class ReadWriteLock
  {
  public:
    ReadWriteLock();
    ~ReadWriteLock();

    void readAcquire();
    void readRelease();
    void writeAcquire();
    void writeRelease();

  private:
    ReadWriteLock(const ReadWriteLock& src);
    ReadWriteLock& operator=(const ReadWriteLock&);

    feabhOS_RWLOCK handle;
  };

} // namespace FeabhOS

#endif // CPP03_FEABHOS_READWRITELOCK_H
