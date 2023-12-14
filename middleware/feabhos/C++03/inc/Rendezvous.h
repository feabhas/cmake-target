// Rendezvous.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef CPP03_FEABHOS_RENDEZVOUS_H
#define CPP03_FEABHOS_RENDEZVOUS_H

#include "feabhOS_rendezvous.h"

// -------------------------------------------------------------------------------------
// The FeabhOS::Rendezvous class provides a C++ wrapper around the
// FeabhOS rendezvous C API.
//
// For details on the semantics of the rendezvous, see the C header
// file.
// -------------------------------------------------------------------------------------

namespace FeabhOS
{
  class Rendezvous
  {
  public:
    Rendezvous();
    ~Rendezvous();

    feabhOS_error call(duration_mSec_t timeout);
    feabhOS_error accept(duration_mSec_t timeout);

  private:
    // Disable copying
    //
    Rendezvous(const Rendezvous&);
    Rendezvous& operator=(const Rendezvous&);

    feabhOS_RENDEZVOUS handle;
  };

} // namespace FeabhOS

#endif
