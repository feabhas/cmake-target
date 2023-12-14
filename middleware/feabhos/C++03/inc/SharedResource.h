// SharedResource.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef CPP03_FEABHOS_SHAREDRESOURCE_H
#define CPP03_FEABHOS_SHAREDRESOURCE_H

#include "Mutex.h"
#include "Condition.h"

#include "feabhOS_time.h"
#include "feabhOS_errors.h"

// -------------------------------------------------------------------------------------
// The Utility::SharedResource class represents a generic thread-safe
// data item.  This class is used as part of the FeabhOS::Mailbox and
// FeabhOS::Promise/Future classes.
//
// write() is non-blocking.
// read() can be blocking or non-blocking
// -------------------------------------------------------------------------------------

namespace FeabhOS
{
  namespace Utility
  {
    template <typename T>
    class SharedResource
    {
    public:
      SharedResource();

      void write(const T& in_val);
      feabhOS_error read(T& inout_val, duration_mSec_t timeout) const;

      bool consumed() const;
      bool set()      const;

    private:
      T value;
      mutable bool is_set;
      mutable bool is_consumed;
      mutable Mutex mutex;
      mutable Condition valueSet;
    };


    template <typename T>
    SharedResource<T>::SharedResource() :
      value(T()),
      is_set(false),
      is_consumed(false),
      mutex(),
      valueSet()
    {
    }


    template <typename T>
    void SharedResource<T>::write(const T& in_val)
    {
      CRITICAL_SECTION(mutex)
      {
        value = in_val;
        is_set = true;
        valueSet.notifyAll();
      }
    }


    template <typename T>
    feabhOS_error SharedResource<T>::read(T& inout_val, duration_mSec_t timeout) const
    {
      CRITICAL_SECTION(mutex)
      {
        while(!is_set)
        {
          // If the condition object timed out then
          // there's no point going round the loop
          // again to see if the data's been set.
          //
          if(valueSet.wait(mutex, timeout) != ERROR_OK)
          {
            return ERROR_TIMED_OUT;
          }
        }
        inout_val    = value;
        is_set       = false;
        is_consumed  = true;
      }
      return ERROR_OK;
    }


    template <typename T>
    bool SharedResource<T>::set() const
    {
      bool set = false;

      CRITICAL_SECTION(mutex)
      {
        set = is_set;
      }
      return set;
    }


    template <typename T>
    bool SharedResource<T>::consumed() const
    {
      bool consumed = false;

      CRITICAL_SECTION(mutex)
      {
        consumed = is_consumed;
      }
      return consumed;
    }

  } // namespace Utility

} // namespace FeabhOS



#endif // CPP03_FEABHOS_SHAREDRESOURCE_H
