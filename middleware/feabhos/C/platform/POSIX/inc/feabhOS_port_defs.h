// feabhOS_port_defs.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef FEABHOS_PORT_DEFS_H
#define FEABHOS_PORT_DEFS_H


//  Include your OS-specific configuration and header
//  files below.
//
#include <pthread.h>
#include <semaphore.h>
#include <limits.h>

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
#define NO_LIMIT                   UINT_MAX

#define MAX_CONDITIONS             NO_LIMIT
#define MAX_EVENTFLAGS             NO_LIMIT
#define MAX_MAILBOXES              NO_LIMIT
#define MAX_MUTEXES                NO_LIMIT
#define MAX_QUEUES                 NO_LIMIT
#define MAX_RENDEZVOUS             NO_LIMIT
#define MAX_RWLOCKS                NO_LIMIT
#define MAX_SEMAPHORES             NO_LIMIT
#define MAX_SIGNALS                NO_LIMIT
#define MAX_TASKS                  NO_LIMIT


// ---------------------------------------------------------------------------
//  Stack size definitions.
//  For your underlying OS define the legitimate stack sizes (in bytes).
//
#define OS_STACK_TINY   	         ((size_bytes_t) 256)
#define OS_STACK_SMALL  	         ((size_bytes_t) 512)
#define OS_STACK_NORMAL 	         ((size_bytes_t)2048)
#define OS_STACK_LARGE  	         ((size_bytes_t)4096)
#define OS_STACK_HUGE   	         ((size_bytes_t)8192)


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
//  For LINUX, this assumes SCH_RR
//
#define OS_PRIORITY_LOWEST         1
#define OS_PRIORITY_LOW            2
#define OS_PRIORITY_NORMAL         3
#define OS_PRIORITY_HIGH           4
#define OS_PRIORITY_HIGHEST        5


// ---------------------------------------------------------------------------
//
//  OS-specific structures
//  ----------------------
//
//  All OSes use specific types to identify OS elements.  These may be simple
//  integer values, structures or pointers-to-opaque-types.  Use the following
//  macros to define the underlying types used by your OS.
//
#define OS_TASK_TYPE               pthread_t
#define OS_MUTEX_TYPE              pthread_mutex_t
#define OS_SIGNAL_TYPE             sem_t
#define OS_CONDITION_TYPE          pthread_cond_t
#define OS_BINARY_SEMAPHORE_TYPE   sem_t
#define OS_COUNTING_SEMAPHORE_TYPE sem_t
#define OS_SEMAPHORE_TYPE          sem_t
#define OS_QUEUE_TYPE              mqd_t
#define OS_MAILBOX_TYPE            mqd_t
#define OS_EVENTFLAGS_TYPE


// ---------------------------------------------------------------------------
//
//  OS-specific data types
//  -----------------------
//
//  Use this macro to define OS-specific signed, unsigned and floating-point
//  types.
//
#define OS_CHAR_TYPE               char
#define OS_FLOAT_TYPE              float
#define OS_DOUBLE_TYPE             double
#define OS_LONG_TYPE               long
#define OS_SHORT_TYPE              short

#define OS_SIGNED_TYPE             int
#define OS_UNSIGNED_TYPE           unsigned


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
//  Note:  for POSIX infinite timeouts are implementation defined. However,
//         the POSIX API supports timed and inifinitely-blocking variants
//         of calls; so OS_INFINITE_TIMEOUT can be used as a function-selector
//
#define OS_TIME_TYPE               (struct timespec)
#define OS_ZERO_TIMEOUT            (0)
#define OS_INFINITE_TIMEOUT        (0xFFFFFFFF)


// ---------------------------------------------------------------------------
//
//  OS-specific error types
//  -----------------------
//
//  Use this macro to define the type returned by your OS in the event of an error.
//  You may extend this list if your OS returns different error types.
//
#define OS_ERROR_TYPE              int

#endif /* FEABHOS_DEFS_H */
