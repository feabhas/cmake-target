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

#include "Semaphore.h"

namespace FeabhOS
{
  Semaphore::Semaphore(num_elements_t maxCount, num_elements_t initCount) :
    handle(NULL)
  {
    feabhOS_error err = feabhOS_semaphore_create(&handle, maxCount, initCount);

    if(err != ERROR_OK)
    {
      // What to do here?!
    }
  }


  Semaphore::Semaphore() :
    handle(NULL)
  {
    feabhOS_error err = feabhOS_semaphore_create(&handle, 1, 0);

    if(err != ERROR_OK)
    {
      // What to do here?!
    }
  }


  Semaphore::~Semaphore()
  {
    feabhOS_semaphore_destroy(&handle);
  }


  feabhOS_error Semaphore::take(duration_mSec_t timeout)
  {
    return feabhOS_semaphore_take(&handle, timeout);
  }


  feabhOS_error Semaphore::give()
  {
    return feabhOS_semaphore_give(&handle);
  }

} // namespace FeabhOS
