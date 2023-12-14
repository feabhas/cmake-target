// Scheduler.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef CPP14_FEABHOS_SCHEDULER_H
#define CPP14_FEABHOS_SCHEDULER_H

#include "feabhOS_scheduler.h"

// -------------------------------------------------------------------------------------
// The FeabhOS::Scheduler class provides a C++ wrapper around the
// FeabhOS scheduler C API.
//
// For details on the semantics of the scheduler, see the C header
// file.
// -------------------------------------------------------------------------------------

namespace FeabhOS {

  class Scheduler {
  public:
    static void init()  { feabhOS_scheduler_init();  }
    static void start() { feabhOS_scheduler_start(); }
  };

} // namespace FeabhOS

#endif // CPP14_FEABHOS_SCHEDULER_H

