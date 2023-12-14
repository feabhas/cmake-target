// Signal.cpp
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#include "Signal.h"

namespace FeabhOS
{
  Signal::Signal() : handle(0)
  {
    feabhOS_error err;

    err = feabhOS_signal_create(&handle);
    if(err != ERROR_OK)
    {
      // What to do?!
    }
  }


  feabhOS_error Signal::wait(duration_mSec_t timeout)
  {
    return feabhOS_signal_wait(&handle, timeout);
  }


  void Signal::notifyOne()
  {
    feabhOS_signal_notify_one(&handle);
  }


  void Signal::notifyAll()
  {
    feabhOS_signal_notify_all(&handle);
  }


  Signal::~Signal()
  {
    feabhOS_signal_destroy(&handle);
  }

} // namespace FeabhOS
