// ---------------------------------------------------------------------------
//
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
// ---------------------------------------------------------------------------

#ifndef FEABHOS_PORT_DEFS_H
#define FEABHOS_PORT_DEFS_H

#include <limits.h>

//  Include your OS-specific configuration and header
//  files below.
//
#include "FreeRTOS.h"
#include "portmacro.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"

// FeabhOS-specific header files
//
#include "feabhOS_stdint.h"

// ---------------------------------------------------------------------------
//
//  Limits for number of OS constructs
//  ----------------------------------
//
//  This macro defines the maximum number of OS objects
//  available.
//
//  For targets that have memory limitations
//  (for example, ARM Cortex M3) use a fixed number
//  and the memory for condition objects will be allocated
//  statically (at compile time).
//
//  Set these values to NO_LIMIT and OS objects will be
//  allocated dynamically (with malloc) at run-time.
//  This setting is only sensible if FeabhOS is running
//  on top of Windows or Posix, or similar.
//
#define NO_LIMIT                 UINT_MAX

#define MAX_CONDITIONS            4
#define MAX_EVENTFLAGS            4
#define MAX_MAILBOXES             4
#define MAX_MUTEXES               4
#define MAX_QUEUES                4
#define MAX_RENDEZVOUS            4
#define MAX_RWLOCKS               4
#define MAX_SEMAPHORES            4
#define MAX_SIGNALS               4
#define MAX_TASKS                 4


// ---------------------------------------------------------------------------
//  Stack size definitions.
//  For your underlying OS define the legitimate stack sizes (in bytes).
//
#define OS_STACK_TINY   	((size_bytes_t) 256)
#define OS_STACK_SMALL  	((size_bytes_t) 512)
#define OS_STACK_NORMAL 	((size_bytes_t)1024)
#define OS_STACK_LARGE  	((size_bytes_t)2048)
#define OS_STACK_HUGE   	((size_bytes_t)4096)


// ---------------------------------------------------------------------------
//  Priority definitions
//  --------------------
//
//  The definition of thread priority varies from OS to OS.  Use
//  these macros to define the range of priorities for your
//  system.
//  Typically you will map the FeabhOS definitions against the
//  range of allowable *application* priorities, rather than
//  the maximum permissible range.  That is, there may be priorities
//  reserved for system tasks - for example, timers or the idle
//  task.
//  You may create additional priority definitions if you need
//  them.
//
#define OS_PRIORITY_LOWEST    tskIDLE_PRIORITY + 1
#define OS_PRIORITY_LOW       tskIDLE_PRIORITY + 2
#define OS_PRIORITY_NORMAL    tskIDLE_PRIORITY + 3
#define OS_PRIORITY_HIGH      tskIDLE_PRIORITY + 4
#define OS_PRIORITY_HIGHEST   tskIDLE_PRIORITY + 5


// ---------------------------------------------------------------------------
//
//  OS-specific structures
//  ----------------------
//
//  All OSes use specific types to identify OS elements.  These may be simple
//  integer values, structures or pointers-to-opaque-types.  Use the following
//  macros to define the underlying types used by your OS.
//
#define OS_TASK_TYPE               TaskHandle_t
#define OS_MUTEX_TYPE              SemaphoreHandle_t
#define OS_SIGNAL_TYPE             SemaphoreHandle_t
#define OS_CONDITION_TYPE
#define OS_BINARY_SEMAPHORE_TYPE   SemaphoreHandle_t
#define OS_COUNTING_SEMAPHORE_TYPE SemaphoreHandle_t
#define OS_QUEUE_TYPE              QueueHandle_t
#define OS_MAILBOX_TYPE            QueueHandle_t
#define OS_EVENTFLAGS_TYPE         EventGroupHandle_t


// ---------------------------------------------------------------------------
//
//  OS-specific data types
//  -----------------------
//
//  Use this macro to define OS-specific signed, unsigned and floating-point
//  types.
//
#define OS_CHAR_TYPE                portCHAR
#define OS_FLOAT_TYPE               portFLOAT
#define OS_DOUBLE_TYPE              portDOUBLE
#define OS_LONG_TYPE                portLONG
#define OS_SHORT_TYPE               portSHORT

#define OS_SIGNED_TYPE              BaseType_t
#define OS_UNSIGNED_TYPE            UBaseType_t


// ---------------------------------------------------------------------------
//
//  OS-specific time durations
//  --------------------------
//
//  Your OS uses its own underlying type for specifying time instants/durations.
//  Use the macro definition below to specify the specific underlying type for
//  your OS.
//  You will also need to specify what constitutes a 'zero' duration and an
//  'infinite' period of time for your particular OS.
//
#define OS_TIME_TYPE            portTickType
#define OS_ZERO_TIMEOUT         0
#define OS_INFINITE_TIMEOUT     portMAX_DELAY



// ---------------------------------------------------------------------------
//
//  OS-specific error types
//  -----------------------
//
//  Use this macro to define the type returned by your OS in the event of an error.
//  You may extend this list if your OS returns different error types.
//
#define OS_ERROR_TYPE           portBASE_TYPE

#endif /* FEABHOS_DEFS_H */
