// Scheduler.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef CPP03_FEABHOS_SCHEDULER_H
#define CPP03_FEABHOS_SCHEDULER_H


// -------------------------------------------------------------------------------------
// The FeabhOS::Scheduler class provides a C++ wrapper around the
// FeabhOS scheduler C API.
//
// For details on the semantics of the scheduler, see the C header
// file.
// -------------------------------------------------------------------------------------

namespace FeabhOS
{
  class Scheduler
  {
  public:
    static void start();
    static void init();

  private:
    Scheduler();
  };
}

#endif // CPP03_FEABHOS_SCHEDULER_H

