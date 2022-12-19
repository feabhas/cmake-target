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

#ifndef CPP14_FEABHOS_RENDEZVOUS_H
#define CPP14_FEABHOS_RENDEZVOUS_H

#include "feabhOS_rendezvous.h"
#include "Duration.h"

// -------------------------------------------------------------------------------------
// The FeabhOS::Rendezvous class provides a C++ wrapper around the
// FeabhOS rendezvous C API.
//
// For details on the semantics of the rendezvous, see the C header
// file.
// -------------------------------------------------------------------------------------

namespace FeabhOS {

  class Rendezvous {
  public:
    Rendezvous();
    ~Rendezvous();

    // call       - Blocking wait for accepter
    // call_for   - Wait with timeout.  A return value
    //              of false indicates the accepter did
    //              not respond before the timeout.
    // accept     - Blocking wait for caller
    // accept_for - Wait with timeout.  A return value
    //              of false indicates the caller did
    //              not respond before the timeout.
    //
    inline void call();
    inline bool call_for(const Time::Duration& timeout);
    inline void accept();
    inline bool accept_for(const Time::Duration& timeout);

    // Copy / move policy
    //
    Rendezvous(const Rendezvous&)            = delete;
    Rendezvous& operator=(const Rendezvous&) = delete;
    Rendezvous(Rendezvous&&)                 = delete;
    Rendezvous& operator=(Rendezvous&&)      = delete;

  private:
    feabhOS_RENDEZVOUS handle { nullptr };
  };


  Rendezvous::Rendezvous()
  {
    auto result = feabhOS_rendezvous_create(&handle);
    if (result != ERROR_OK) {
      // What to do here?
    }
  }


  Rendezvous::~Rendezvous()
  {
    feabhOS_rendezvous_destroy(&handle);
  }


  void Rendezvous::call()
  {
    call_for(Time::wait_forever);
  }


  bool Rendezvous::call_for(const Time::Duration& timeout)
  {
    return (feabhOS_rendezvous_call(&handle, timeout) == ERROR_OK);
  }


  void Rendezvous::accept()
  {
    accept_for(Time::wait_forever);
  }


  bool Rendezvous::accept_for(const Time::Duration& timeout)
  {
    return (feabhOS_rendezvous_accept(&handle, timeout) == ERROR_OK);
  }

} // namespace FeabhOS

#endif // CPP14_FEABHOS_RENDEZVOUS_H
