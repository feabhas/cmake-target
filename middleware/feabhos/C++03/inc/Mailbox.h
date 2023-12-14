// Mailbox.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
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
