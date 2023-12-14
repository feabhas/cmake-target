// Signal.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef CPP03_FEABHOS_SIGNAL_H
#define CPP03_FEABHOS_SIGNAL_H

#include "feabhOS_signal.h"
#include "feabhOS_errors.h"

// -------------------------------------------------------------------------------------
// The FeabhOS::Signal class provides a C++ wrapper around the
// FeabhOS signal C API.
//
// For details on the semantics of the signal, see the C header
// file.
// -------------------------------------------------------------------------------------

namespace FeabhOS
{
  class Signal
  {
  public:
    Signal();
    ~Signal();

    feabhOS_error wait(duration_mSec_t timeout);
    void          notifyOne();
    void          notifyAll();

  private:
    feabhOS_SIGNAL handle;

    // Do not allow copying
    //
    Signal(const Signal&);
    Signal& operator=(const Signal&);
  };

} // namespace FeabhOS


#endif // CPP03_FEABHOS_SIGNAL_H
