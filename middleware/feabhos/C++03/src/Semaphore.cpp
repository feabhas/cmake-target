// Semaphore.cpp
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

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
