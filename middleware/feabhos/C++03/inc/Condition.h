// Condition.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef CPP03_FEABHOS_CONDITION_H
#define CPP03_FEABHOS_CONDITION_H

#include "Signal.h"

// -------------------------------------------------------------------------------------
// The FeabhOS::Condition class provides an adapter around the
// FeabhOS::Signal class, with the ability to unlock/lock a
// FeabhOS::Mutex object on wait().
// -------------------------------------------------------------------------------------

namespace FeabhOS
{
  class Mutex;

  class Condition
  {
  public:
    Condition();

    feabhOS_error wait(Mutex& mutex, duration_mSec_t timeout);
    void          notifyOne();
    void          notifyAll();

  private:
    Signal signal;

    // Disable copying
    //
    Condition(const Condition&);
    Condition& operator=(const Condition&);
  };

} // namespace FeabhOS


#endif // CPP03_FEABHOS_CONDITION_H
