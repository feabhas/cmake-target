// Semaphore.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef CPP14_FEABHOS_SEMAPHORE_H
#define CPP14_FEABHOS_SEMAPHORE_H

#include "feabhOS_semaphore.h"
#include "Duration.h"

// -------------------------------------------------------------------------------------
// The FeabhOS::Semaphore class provides a C++ wrapper around the
// FeabhOS counting semaphore C API.
//
// For details on the semantics of the semaphore, see the C header
// file.
// -------------------------------------------------------------------------------------

namespace FeabhOS {

  class Semaphore {
  public:
    // The default constructor sets maxCount => 1, initCount => 0
    // - this is equivalent to a binary semaphore (in the
    // un-taken state)
    //
    Semaphore();
    Semaphore(num_elements_t max_count, num_elements_t init_count);
    ~Semaphore();

    // take()         - Blocking call; will block forever to
    //                  obtain semaphore
    // try_take()     - Non-blocking; will return false if semaphore
    //                  cannot be obtained.
    // try_take_for() - Blocking call; will wait for semaphore until
    //                  timeout expires; returns false if timeout occurred.
    // give()         - Non-blocking; will return true if semaphore is
    //                  given; false if max_count has been reached.
    //
    inline void take();
    inline bool try_take();
    inline bool take_for(const Time::Duration& timeout);
    inline bool give();

    // Copy / move policy
    //
    Semaphore(const Semaphore&)            = delete;
    Semaphore& operator=(const Semaphore&) = delete;
    Semaphore(Semaphore&&)                 = delete;
    Semaphore& operator=(Semaphore&&)      = delete;

  private:
    feabhOS_SEMAPHORE handle { nullptr };
  };


  Semaphore::Semaphore(num_elements_t max_count, num_elements_t init_count)
  {
    auto result = feabhOS_semaphore_create(&handle, max_count, init_count);

    if (result != ERROR_OK) {
      // What to do here?!
    }
  }


  Semaphore::Semaphore() : Semaphore { 1, 0 }
  {
  }


  Semaphore::~Semaphore()
  {
    feabhOS_semaphore_destroy(&handle);
  }


  void Semaphore::take()
  {
    take_for(Time::wait_forever);
  }


  bool Semaphore::try_take()
  {
    return take_for(Time::no_wait);
  }


  bool Semaphore::take_for(const Time::Duration& timeout)
  {
    return (feabhOS_semaphore_take(&handle, timeout) == ERROR_OK);
  }


  bool Semaphore::give()
  {
    return (feabhOS_semaphore_give(&handle) == ERROR_OK);
  }

} // namespace FeabhOS

#endif // CPP03_FEABHOS_SEMAPHORE_H
