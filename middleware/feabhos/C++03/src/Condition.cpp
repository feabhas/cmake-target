// Condition.cpp
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#include "Condition.h"
#include "Mutex.h"
#include "Signal.h"

namespace FeabhOS
{
  Condition::Condition() :
    signal()
  {
  }


  feabhOS_error Condition::wait(Mutex& mutex, duration_mSec_t timeout)
  {
    feabhOS_error err;
    mutex.unlock();
    err = signal.wait(timeout);
    mutex.lock(WAIT_FOREVER);
    return err;
  }


  void Condition::notifyAll()
  {
    signal.notifyAll();
  }


  void Condition::notifyOne()
  {
    signal.notifyOne();
  }

} // namespace FeabhOS
