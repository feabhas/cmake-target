// Signal.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
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
