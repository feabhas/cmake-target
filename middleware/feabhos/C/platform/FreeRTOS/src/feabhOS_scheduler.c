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

#include <stdbool.h>
#include "feabhOS_scheduler.h"
#include "feabhOS_memory.h"
#include "FreeRTOS.h"
#include "task.h"


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
bool scheduler_started = false;


feabhOS_error feabhOS_scheduler_init(void)
{
  feabhOS_memory_init();
  return ERROR_OK;
}


feabhOS_error feabhOS_scheduler_start(void)
{
  scheduler_started = true;
  vTaskStartScheduler();
  return ERROR_OK;
}
