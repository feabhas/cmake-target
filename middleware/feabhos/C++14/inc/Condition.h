// Condition.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef CPP14_FEABHOS_CONDITION_H
#define CPP14_FEABHOS_CONDITION_H

#include "Signal.h"
#include "Mutex.h"

// -------------------------------------------------------------------------------------
// The FeabhOS::Condition class provides a C++ wrapper around the
// FeabhOS condition variable C API.
//
// -------------------------------------------------------------------------------------

namespace FeabhOS {

  namespace Time { class Duration; }
  class Mutex;

  class Condition {
  public:
    Condition() = default;

    // Condition API:
    // wait()       - Blocking call; will block forever for
    //                condition to be signalled.
    // wait_for()   - Blocking call; will wait for condition
    //                until timeout expires
    // wait_while() - Block while predicate is true
    //
    inline void wait(Mutex& mutex);
    inline bool wait_for(Mutex& mutex, const Time::Duration& timeout);

    template <typename Pred_Ty>
    inline void wait_while(Mutex& mutex, const Pred_Ty&& predicate);

    // notify_one() - Release one thread.
    // notify_all() - Release all waiting threads
    //
    inline void notify_one();
    inline void notify_all();

    // Copy / move policy
    //
    Condition(const Condition&)            = delete;
    Condition& operator=(const Condition&) = delete;
    Condition(Condition&&)                 = delete;
    Condition& operator=(Condition&&)      = delete;

  private:
    Signal signal { };
  };


  void Condition::wait(Mutex& mutex)
  {
    wait_for(mutex, Time::wait_forever);
  }


  bool Condition::wait_for(Mutex& mutex, const Time::Duration& timeout)
  {
    bool timed_out { };
    mutex.unlock();
    timed_out = signal.wait_for(timeout);
    mutex.lock();
    return timed_out;
  }


  template <typename Pred_Ty>
  void Condition::wait_while(Mutex& mutex, const Pred_Ty&& predicate)
  {
    while (predicate()) wait(mutex);
  }


  void Condition::notify_one()
  {
    signal.notify_one();
  }


  void Condition::notify_all()
  {
    signal.notify_all();
  }

} // namespace FeabhOS

#endif // CPP14_FEABHOS_CONDITION_H
