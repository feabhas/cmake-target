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

#ifndef CPP14_FEABHOS_SHAREDRESOURCE_H
#define CPP14_FEABHOS_SHAREDRESOURCE_H

#include "Mutex.h"
#include "Condition.h"
#include "Duration.h"

// -------------------------------------------------------------------------------------
// The Utility::SharedResource class represents a generic thread-safe
// data item.  This class is used as part of the FeabhOS::Mailbox and
// FeabhOS::Promise/Future classes.
//
// set() is non-blocking.
// get() can be blocking or non-blocking
// -------------------------------------------------------------------------------------

namespace FeabhOS {

  namespace Utility {

    template <typename T>
    class SharedResource {
    public:
      SharedResource() = default;
      SharedResource(const T& init);

      // set()         - Non-blocking.  Will overwrite old value.
      // get()         - Blocking call; will block forever to
      //                 obtain value
      // try_get()     - Non-blocking; will return false if value
      //                 cannot be obtained.
      // try_get_for() - Blocking call; will wait for value until
      //                 timeout expires
      //
      // Set supports moving and copying; get methods will always
      // move from the internal value.
      //
      template <typename U>
      void set(U&& in_val);

      void get(T& inout_val) const;
      bool try_get(T& inout_val) const;
      bool try_get_for(T& inout_val, const Time::Duration& timeout) const;

      // Query the state of the shared object.  A value cannot be
      // both set and consumed.
      bool is_consumed() const;
      bool is_set()      const;

      // Copy / move policy.
      // Since Mutexes and Conditions cannot
      // be moved or copied, neither can
      // SharedResources
      //
      SharedResource(const SharedResource&)            = delete;
      SharedResource& operator=(const SharedResource&) = delete;
      SharedResource(SharedResource&&)                 = delete;
      SharedResource& operator=(SharedResource&&)      = delete;

    private:
      T value                     { };
      mutable bool value_set      { false };
      mutable bool value_consumed { false };
      mutable Mutex mutex         { };
      mutable Condition set_cond  { };
    };


    template <typename T>
    SharedResource<T>::SharedResource(const T& init) : value { init }
    {
    }


    template <typename T>
    template <typename U>
    void SharedResource<T>::set(U&& in_val)
    {
      CRITICAL_SECTION(mutex)
      {
        value          = std::forward<U>(in_val);
        value_set      = true;
        value_consumed = false;
        set_cond.notify_all();
      }
    }


    template <typename T>
    void SharedResource<T>::get(T& inout_val) const
    {
      try_get_for(inout_val, Time::wait_forever);
    }


    template <typename T>
    bool SharedResource<T>::try_get(T& inout_val) const
    {
      return try_get_for(inout_val, Time::no_wait);
    }


    template <typename T>
    bool SharedResource<T>::try_get_for(T& inout_val, const Time::Duration& timeout) const
    {
      CRITICAL_SECTION(mutex)
      {
        while(!value_set) {
          // If the condition object timed out then
          // there's no point going round the loop
          // again to see if the data's been set.
          //
          if (set_cond.wait_for(mutex, timeout) != ERROR_OK) {
            return false;
          }
        }
        inout_val      = std::move(value);
        value_set      = false;
        value_consumed = true;
      }
      return true;
    }


    template <typename T>
    bool SharedResource<T>::is_set() const
    {
      bool set = false;

      CRITICAL_SECTION(mutex) { set = value_set; }
      return set;
    }


    template <typename T>
    bool SharedResource<T>::is_consumed() const
    {
      bool consumed = false;

      CRITICAL_SECTION(mutex) { consumed = value_consumed; }
      return consumed;
    }

  } // namespace Utility

} // namespace FeabhOS



#endif // CPP14_FEABHOS_SHAREDRESOURCE_H
