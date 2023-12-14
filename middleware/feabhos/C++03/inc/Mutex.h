// Mutex.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef CPP03_FEABHOS_MUTEX_H
#define CPP03_FEABHOS_MUTEX_H

#include "feabhOS_mutex.h"
#include "feabhOS_time.h"

// -------------------------------------------------------------------------------------
// The FeabhOS::Mutex class provides a C++ wrapper around the
// FeabhOS mutex C API.
//
// For details on the semantics of the mutex, see the C header
// file.
// -------------------------------------------------------------------------------------

namespace FeabhOS
{
  //
  class Mutex
  {
  public:
    Mutex();
    ~Mutex();
    feabhOS_error lock(duration_mSec_t timeout);
    void          unlock();

  private:
    Mutex(const Mutex&);
    Mutex& operator=(const Mutex&);

    // feabhOS-specific implementation
    //
    feabhOS_MUTEX handle;
  };


  // -------------------------------------------------------------------------------------
  // ScopeLock implements the
  // scope-locked idiom for
  // mutexes.
  //
  class ScopeLock
  {
  public:
    ScopeLock(Mutex& mtx) : mutex(mtx) { mutex.lock(WAIT_FOREVER); }
    ~ScopeLock()                       { mutex.unlock(); }
    operator bool()                    { return true; }

  private:
    Mutex& mutex;
  };

} // namespace FeabhOS


// -------------------------------------------------------------------------------------
// Neat little macro to simplify code.
// Requires the ScopeLock class to implement
// operator bool()
//
#define CRITICAL_SECTION(mtx) if(FeabhOS::ScopeLock _localLock_ = mtx)


#endif // CPP_FEABHOS_MUTEX_H
