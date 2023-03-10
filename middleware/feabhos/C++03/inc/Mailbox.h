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

#ifndef CPP03_FEABHOS_MAILBOX_H
#define CPP03_FEABHOS_MAILBOX_H

#include "SharedResource.h"
#include "feabhOS_time.h"

// -------------------------------------------------------------------------------------
// The FeabhOS::Mailbox class provides an adapter around the
// FeabhOS::Utility::SharedResource class, providing a message
// queue-like interface.
// -------------------------------------------------------------------------------------

namespace FeabhOS
{
  template <typename T>
  class Mailbox
  {
  public:
    Mailbox();

    inline void post(const T& in_val);
    inline feabhOS_error get(T& inout_val, duration_mSec_t timeout);
    inline bool isEmpty() const;

  private:
    Mailbox(const Mailbox&);
    Mailbox operator=(const Mailbox&);

    Utility::SharedResource<T> box;
  };


  template <typename T>
  Mailbox<T>::Mailbox() : box()
  {
  }


  template <typename T>
  void Mailbox<T>::post(const T& in_val)
  {
    box.set(in_val);
  }


  template <typename T>
  feabhOS_error Mailbox<T>::get(T& inout_val, duration_mSec_t timeout)
  {
    return box.get(inout_val, timeout);
  }


  template <typename T>
  bool Mailbox<T>::isEmpty() const
  {
    return !box.isSet();
  }

}


#endif // CPP03_FEABHOS_MAILBOX_H
