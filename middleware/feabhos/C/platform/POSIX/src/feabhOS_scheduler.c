// feabhOS_scheduler.c
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#include <stdbool.h>
#include "feabhOS_scheduler.h"
#include "feabhOS_memory.h"


// This global variable can be queried by
// other modules to know if the underlying
// OS is actually running.
// For example, making a blocking call before
// the OS has started will block the whole
// system!
// For typical RTOS implementations (FreeRTOS,
// EmbOS, uC-OS, etc) this should be set to
// false until the call to scheduler_start().
// For hosted-OS platforms (Windows, Linux)
// this can be set to true.
//
bool scheduler_started = true;


feabhOS_error feabhOS_scheduler_init(void)
{
  feabhOS_memory_init();
  return ERROR_OK;
}


feabhOS_error feabhOS_scheduler_start(void)
{
  scheduler_started = true;
  return ERROR_OK;
}
