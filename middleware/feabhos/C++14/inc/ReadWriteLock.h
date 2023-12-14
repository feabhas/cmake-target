// ReadWriteLock.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef CPP14_FEABHOS_READWRITELOCK_H
#define CPP14_FEABHOS_READWRITELOCK_H

#include "feabhOS_rwlock.h"

namespace FeabhOS {

  class ReadWriteLock {
  public:
    ReadWriteLock();
    ~ReadWriteLock();

    inline void read_acquire();
    inline void read_release();
    inline void write_acquire();
    inline void write_release();

    // Copy / move policy
    //
    ReadWriteLock(const ReadWriteLock& src)        = delete;
    ReadWriteLock& operator=(const ReadWriteLock&) = delete;
    ReadWriteLock(ReadWriteLock&& src)             = delete;
    ReadWriteLock& operator=(ReadWriteLock&&)      = delete;

  private:
    feabhOS_RWLOCK handle { nullptr };
  };


  ReadWriteLock::ReadWriteLock()
  {
    auto result = feabhOS_rwlock_create(&handle);
    if (result != ERROR_OK) {
      // What to do here?!
    }
  }


  ReadWriteLock::~ReadWriteLock()
  {
    feabhOS_rwlock_destroy(&handle);
  }


  void ReadWriteLock::read_acquire()
  {
    feabhOS_rwlock_read_acquire(&handle);
  }


  void ReadWriteLock::read_release()
  {
    feabhOS_rwlock_read_release(&handle);
  }


  void ReadWriteLock::write_acquire()
  {
    feabhOS_rwlock_write_acquire(&handle);
  }


  void ReadWriteLock::write_release()
  {
    feabhOS_rwlock_write_release(&handle);
  }

} // namespace FeabhOS

#endif // CPP14_FEABHOS_READWRITELOCK_H
