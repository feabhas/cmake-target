// Semaphore.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef CPP03_FEABHOS_SEMAPHORE_H
#define CPP03_FEABHOS_SEMAPHORE_H

#include "feabhOS_semaphore.h"

// -------------------------------------------------------------------------------------
// The FeabhOS::Semaphore class provides a C++ wrapper around the
// FeabhOS counting semaphore C API.
//
// For details on the semantics of the semaphore, see the C header
// file.
// -------------------------------------------------------------------------------------

namespace FeabhOS
{
  class Semaphore
  {
  public:
    // The default constructor sets maxCount => 1, initCount => 0
    // - this is equivalent to a binary semaphore (in the
    // un-taken state)
    //
    Semaphore();
    Semaphore(num_elements_t maxCount, num_elements_t initCount);
    ~Semaphore();

    feabhOS_error take(duration_mSec_t timeout);
    feabhOS_error give();

  private:
    Semaphore(const Semaphore&);
    Semaphore& operator=(const Semaphore&);

    feabhOS_SEMAPHORE handle;
  };

} // namespace FeabhOS

#endif // CPP03_FEABHOS_SEMAPHORE_H
