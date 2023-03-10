// -------------------------------------------------------------------------------------
//  FeabhOS OS abstraction layer
//
//  DISCLAIMER:
//  Feabhas is furnishing this item "as is". Feabhas does not provide any warranty
//  of the item whatsoever, whether express, implied, or statutory, including, but
//  not limited to, any warranty of merchantability or fitness for a particular
//  purpose or any warranty that the contents of the item will be error-free.
//  In no respect shall Feabhas incur any liability for any damages, including, but
//  limited to, direct, indirect, special, or consequential damages arising out of,
//  resulting from, or any way connected to the use of the item, whether or not
//  based upon warranty, contract, tort, or otherwise; whether or not injury was
//  sustained by persons or property or otherwise; and whether or not loss was
//  sustained from, or arose out of, the results of, the item, or any services that
//  may be provided by Feabhas.
//
// -------------------------------------------------------------------------------------

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
