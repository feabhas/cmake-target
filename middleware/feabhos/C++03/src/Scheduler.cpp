// Scheduler.cpp
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#include "Scheduler.h"
#include "feabhOS_scheduler.h"

namespace FeabhOS
{
  void Scheduler::start()
  {
    feabhOS_scheduler_start();
  }


  void Scheduler::init()
  {
    feabhOS_scheduler_init();
  }
}
