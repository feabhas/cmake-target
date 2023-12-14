// Rendezvous.cpp
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#include <cstddef>
#include "Rendezvous.h"

namespace FeabhOS
{
  Rendezvous::Rendezvous() :
    handle(NULL)
  {
      feabhOS_error error = feabhOS_rendezvous_create(&handle);
      if(error != ERROR_OK)
      {
        // What to do here?
      }
  }


  Rendezvous::~Rendezvous()
  {
    feabhOS_rendezvous_destroy(&handle);
  }


  feabhOS_error Rendezvous::call(duration_mSec_t timeout)
  {
    return feabhOS_rendezvous_call(&handle, timeout);
  }


  feabhOS_error Rendezvous::accept(duration_mSec_t timeout)
  {
    return feabhOS_rendezvous_accept(&handle, timeout);
  }

} // namespace FeabhOS
