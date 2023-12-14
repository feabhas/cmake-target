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
#include <cstddef>
#include "SharedResource.h"

namespace FeabhOS
{
  // --------------------------------------------
  // Exceptions for futures
  //
  class FutureUsed : public std::logic_error
  {
  public:
    FutureUsed() : logic_error("Future already consumed") { }
  };


  class FutureEmpty : public std::logic_error
  {
  public:
    FutureEmpty() : logic_error("Future uninitialised") { }
  };


  class PromiseSet : public std::logic_error
  {
  public:
    PromiseSet() : logic_error("Promise already set") { }
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
    Future() : promise(NULL) { }

    // Blocking get.  Returns a copy
    // of the data object.
    //
    T get() const;
    operator T() const;

    // Non-blocking get
    //
    bool tryGet(T& inout_val) const;
    bool isReady() const;

  protected:
    friend class Promise<T>;
    Future(Promise<T>& parent) : promise(&parent) { }

  private:
    Promise<T>* promise;
  };


  // --------------------------------------------
  // Promise provides the write-only interface
  // to the data object
  //
  template <typename T>
  class Promise
  {
  public:
    Promise();

    // Set copies the input value into
    // the data object
    //
    void set(const T& in_val);

    Future<T> getFuture();

  private:
    friend class Future<T>;
    Utility::SharedResource<T> data;
  };



  template <typename T>
  T Future<T>::get() const
  {
    // You can't get from an uninitialised Future as
    // it doesn't belong to any promise.
    // Also, you can only read from the Future once.
    //
    if(promise == NULL)          throw FutureEmpty();
    if(promise->data.consumed()) throw FutureUsed();

    T temp = T();
    promise->data.read(temp, WAIT_FOREVER);
    return temp;
  }


  template <typename T>
  Future<T>::operator T() const
  {
    return get();
  }


  template <typename T>
  bool Future<T>::tryGet(T& inout_val) const
  {
    if(!promise->data.set()) return false;
    return (promise->data.read(inout_val, NO_WAIT) == ERROR_OK);
  }


  template <typename T>
  bool Future<T>::isReady() const
  {
    return (promise->data.set());
  }


  template <typename T>
  void Promise<T>::set(const T& in_val)
  {
    // You can't set a promise twice;
    // nor can you re-use a promise
    //
    if(data.set())      throw PromiseSet();
    if(data.consumed()) throw FutureUsed();
    data.write(in_val);
  }


  template <typename T>
  Promise<T>::Promise() :
    data()
  {
  }


  template <typename T>
  Future<T> Promise<T>::getFuture()
  {
    return Future<T>(*this);
  }


} // namespace FeabhOS

#endif // CPP03_FEABHOS_FUTURE_H
