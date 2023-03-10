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
