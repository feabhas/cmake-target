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
