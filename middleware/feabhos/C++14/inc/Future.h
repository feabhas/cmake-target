// Future.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef CPP03_FEABHOS_FUTURE_H
#define CPP03_FEABHOS_FUTURE_H

// -------------------------------------------------------------------------------------
// The FeabhOS::Promise and FeabhOS::Future classes provides an
// implementation of the Deferred Synchronous Call pattern.
// The Promise provides the write-only interface to a shared state
// object; the Future provides the read-only interface.
// -------------------------------------------------------------------------------------

#include <stdexcept>
#include <exception>
#include "SharedResource.h"

namespace FeabhOS {

  // --------------------------------------------
  // Exceptions for futures
  //
  class future_used : public std::logic_error
  {
  public:
    future_used() : logic_error("Future already consumed!") { }
    future_used(const char* str) : logic_error(str) { }
  };


  class future_empty : public std::logic_error
  {
  public:
    future_empty() : logic_error("Future uninitialised!") { }
    future_empty(const char* str) : logic_error(str) { }
  };


  class promise_set : public std::logic_error
  {
  public:
    promise_set() : logic_error("Promise already set!") { }
    promise_set(const char* str) : logic_error(str) { }
  };


  class exception_set : public std::logic_error
  {
  public:
    exception_set() : logic_error("Promise exception already set!") { }
    exception_set(const char* str) : logic_error(str) { }
  };


  // --------------------------------------------
  // Future provides the read-only interface
  // to the data object
  //
  template <typename T> class Promise;

  template <typename T>
  class Future
  {
  public:
    Future() = default;

    // get()         - Blocking call; will block forever to
    //                 obtain value
    // operator T()  - Same as get().
    // try_get()     - Non-blocking; will return false if value
    //                 cannot be obtained.
    // try_get_for() - Blocking call; will wait for value until
    //                 timeout expires
    //
    // Notice, the get() calls are not marked const.  This is
    // because reading a Future 'consumes' it, effectively
    // modifying the state of the object.
    //
    // If an exception has been set (rather than a value) on
    // the owning Promise then that exception will be thrown
    // in the Futures context (thread).
    //
    T get();
    operator T();
    bool try_get(T& inout_val);
    bool try_get_for(T& inout_val, const Time::Duration& timeout);

    // Non-modifying inspector functions
    //
    bool is_ready()   const;
    bool is_expired() const;

  protected:
    friend class Promise<T>;
    Future(Promise<T>& parent) : promise { &parent } { }

  private:
    Promise<T>* promise { nullptr };
  };


  // --------------------------------------------
  // Promise provides the write-only interface
  // to the data object
  //
  template <typename T>
  class Promise {
  public:

    // Set copies or moves the input value into
    // the data object.
    // Will throw exception_set if an
    // exception has already been set.
    //
    template <typename U>
    void set(U&& in_val);

    // Return an exception to the client.
    // When the client calls any of the get()
    // functions on their Future the exception
    // will be thrown in their context (thread).
    // Setting an exception is mutually-exclusive
    // with setting a value. The call will throw
    // promise_set if set() has already been called.
    //
    template <typename Exception_Ty>
    void set_exception(Exception_Ty&& ex);

    // Returns a Future object bound to this
    // Promise.
    //
    Future<T> get_future();

  private:
    friend class Future<T>;
    Utility::SharedResource<T> value   { };
    std::exception_ptr         exc_ptr { nullptr };
  };


  template <typename T>
  Future<T>::operator T()
  {
    return get();
  }


  template <typename T>
  T Future<T>::get()
  {
    T temp { };
    try_get_for(temp, Time::wait_forever);
    return temp;
  }


  template <typename T>
  bool Future<T>::try_get(T& inout_val)
  {
    return try_get_for(inout_val, Time::no_wait);
  }


  template <typename T>
  bool Future<T>::try_get_for(T& inout_val, const Time::Duration& timeout)
  {
    // You can't get() from an uninitialized Future as
    // it doesn't belong to any promise.
    // Also, you can only read from the Future once.
    //
    if (!promise)                     return false;
    if (promise->value.is_consumed()) throw future_used { };
    if (promise->exc_ptr)             std::rethrow_exception(promise->exc_ptr);

    return (promise->value.try_get_for(inout_val, timeout));
  }


  template <typename T>
  bool Future<T>::is_ready() const
  {
    // An uninitialized Future is never ready
    //
    return (promise) ? promise->value.is_set() : false;
  }


  template <typename T>
  bool Future<T>::is_expired() const
  {
    // An uninitialized Future hasn't been consumed.
    //
    return (promise) ? promise->value.is_consumed() : false;
  }


  template <typename T>
  template <typename U>
  void Promise<T>::set(U&& in_val)
  {
    if (value.is_set())      throw promise_set   { };
    if (value.is_consumed()) throw future_used   { };
    if (exc_ptr)             throw exception_set { };

    value.set(std::forward<U>(in_val));
  }


  template <typename T>
  template <typename Exception_Ty>
  void Promise<T>::set_exception(Exception_Ty&& ex)
  {
    if (value.is_set()) throw promise_set { };

    try {
      throw Exception_Ty { std::forward<Exception_Ty>(ex) };
    }
    catch (...) {
      exc_ptr = std::current_exception();
    }
  }


  template <typename T>
  Future<T> Promise<T>::get_future()
  {
    return Future<T>(*this);
  }


} // namespace FeabhOS

#endif // CPP03_FEABHOS_FUTURE_H
