// Mutex.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef CPP14_FEABHOS_MUTEX_H
#define CPP14_FEABHOS_MUTEX_H

#include "feabhOS_mutex.h"
#include "Duration.h"

// -------------------------------------------------------------------------------------
// The FeabhOS::Mutex class provides a C++ wrapper around the
// FeabhOS mutex C API.
//
// -------------------------------------------------------------------------------------

namespace FeabhOS {

  namespace Time { class Duration; }

  class Mutex {
  public:
    inline Mutex();
    inline ~Mutex();

    // Lock API:
    // lock()         - Blocking call; will block forever to
    //                  obtain mutex
    // try_lock()     - Non-blocking; will return false if lock
    //                  cannot be obtained.
    // try_lock_for() - Blocking call; will wait for lock until
    //                  timeout expires
    //
    inline void lock();
    inline bool try_lock();
    inline bool try_lock_for(const Time::Duration& timeout);
    inline void unlock();

    // Copy / move policy
    //
    Mutex(const Mutex&)             = delete;
    Mutex& operator=(const Mutex&)  = delete;
    Mutex(Mutex&&)                  = delete;
    Mutex& operator=(Mutex&&)       = delete;

private:
    feabhOS_MUTEX handle { nullptr };
  };


  Mutex::Mutex()
  {
    auto err = feabhOS_mutex_create(&handle);
    if (err != ERROR_OK) {
      // What to do here?
    }
  }


  void Mutex::lock()
  {
    try_lock_for(Time::wait_forever);
  }


  bool Mutex::try_lock()
  {
    return try_lock_for(Time::no_wait);
  }


  bool Mutex::try_lock_for(const Time::Duration& timeout)
  {
    return (feabhOS_mutex_lock(&handle, timeout) == ERROR_OK);
  }


  void Mutex::unlock()
  {
    feabhOS_mutex_unlock(&handle);
  }


  Mutex::~Mutex()
  {
    feabhOS_mutex_destroy(&handle);
  }


  // ---------------------------------------------------
  // ScopeLock implements the scope-locked idiom for
  // mutexes.
  //

  class ScopeLock {
  public:
    explicit ScopeLock(Mutex& mtx) : mutex(mtx) { mutex.lock(); }
    ~ScopeLock()                                { mutex.unlock(); }
    operator bool() const                       { return true; }

  private:
    Mutex& mutex;
  };

} // namespace FeabhOS

// Neat little macro to simplify code.
// Requires the ScopeLock class to implement
// operator bool()
//
#define CRITICAL_SECTION(mtx) if(FeabhOS::ScopeLock&& _lock_ = FeabhOS::ScopeLock { mtx })



#endif // CPP14_FEABHOS_MUTEX_H
