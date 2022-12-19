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
#ifndef CPP14_FEABHOS_SIGNAL_H
#define CPP14_FEABHOS_SIGNAL_H

#include "feabhOS_signal.h"
#include "Duration.h"

// -------------------------------------------------------------------------------------
// The FeabhOS::Signal class provides a C++ wrapper around the
// FeabhOS signal C API.
//
// -------------------------------------------------------------------------------------

namespace FeabhOS {

  namespace Time { class Duration; }

  class Signal {
  public:
    inline Signal();
    inline ~Signal();

    // Signal API:
    // wait()     - Blocking call; will block forever for
    //              signal to be signalled.
    // wait_for() - Blocking call; will wait for signal
    //              until timeout expires
    //
    inline void wait();
    inline bool wait_for(const Time::Duration& timeout);

    // notify_one() - Release one thread.
    // notify_all() - Release all waiting threads
    //
    inline void notify_one();
    inline void notify_all();
  
    // Copy / move policy
    //
    Signal(const Signal&)            = delete;
    Signal& operator=(const Signal&) = delete;
    Signal(Signal&&)                 = delete;
    Signal& operator=(Signal&&)      = delete;

  private:
    feabhOS_SIGNAL handle { nullptr };
  };


  Signal::Signal()
  {
    auto err = feabhOS_signal_create(&handle);
    if (err != ERROR_OK) {
      // What to do?!
    }
  }


  void Signal::wait()
  {
    wait_for(Time::wait_forever);
  }


  bool Signal::wait_for(const Time::Duration& timeout)
  {
    return (feabhOS_signal_wait(&handle, timeout) == ERROR_OK);
  }


  void Signal::notify_one()
  {
    feabhOS_signal_notify_one(&handle);
  }


  void Signal::notify_all()
  {
    feabhOS_signal_notify_all(&handle);
  }


  Signal::~Signal()
  {
    feabhOS_signal_destroy(&handle);
  }

} // namespace FeabhOS

#endif // CPP14_FEABHOS_SIGNAL_H
