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
