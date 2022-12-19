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

#include <assert.h>
#include <stdbool.h>
#include <errno.h>
#include <semaphore.h>
#include "feabhOS_signal.h"
#include "feabhOS_port_defs.h"
#include "feabhOS_time_utils.h"


// ----------------------------------------------------------------------------
//  NOTE:
//  POSIX does not support the idea of a
//  task-independent signal, so we must
//  simulate it with a binary semaphore
//  (The 'semaphore-as-signal' pattern)
//
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Making a blocking call before the scheduler is started
// could lock up the entire system.  Therefore, any attempt
// to make a blocking call should assert.
//
extern bool scheduler_started;

// ----------------------------------------------------------------------------
// Management structure
//
struct feabhOS_signal
{
  OS_BINARY_SEMAPHORE_TYPE handle;
  num_elements_t           waiting_tasks;
  
};

// ----------------------------------------------------------------------------
//
//  MEMORY MANAGEMENT FOR SIGNAL STRUCTURES
//  ---------------------------------------
//
//  For a fixed number of signals we use a fixed-block
//  dynamic allocator.
//  If MAX_TASKS == NO_LIMIT we use the underlying OS'
//  dynamic memory allocator (usually malloc)
//

#if MAX_SIGNALS==NO_LIMIT

#include "feabhOS_memory.h"

static feabhOS_SIGNAL allocate(void)
{
  return feabhOS_memory_alloc(sizeof(struct feabhOS_signal));
}


static void deallocate(feabhOS_SIGNAL signal)
{
  feabhOS_memory_free(signal);
}

#else

#include "feabhOS_allocator.h"
#include "feabhOS_task.h"
#include "feabhOS_condition.h"
#include "feabhOS_rwlock.h"

// Every task has a signal attached to it (for joining).
// Every condition object is essentially a signal.
// Each reader-writer lock requires two conditions.
//
// So, to ensure we don't run out of signals
//
// TOTAL_SIGNALS = MAX_TASKS  +
//                 MAX_CONDITIONS +
//                 MAX_RWLOCKS * 2 +
//                 MAX (USER) SIGNALS
//
// This is only relevant for static (fixed-block) allocation.
//
#define TOTAL_SIGNALS (MAX_TASKS + MAX_CONDITIONS + (MAX_RWLOCKS * 2) + MAX_SIGNALS)

static struct feabhOS_signal signals[TOTAL_SIGNALS];
static feabhOS_POOL signal_pool = NULL;


static feabhOS_SIGNAL allocate(void)
{
  // Craete the pool first time a task is allocated
  //
  if(signal_pool == NULL)
  {
    feabhOS_pool_create(&signal_pool,
                        signals,
                        sizeof(signals),
                        sizeof(struct feabhOS_signal),
                        MAX_SIGNALS);
  }

  return feabhOS_block_allocate(&signal_pool);
}


static void deallocate(feabhOS_SIGNAL signal)
{
  feabhOS_block_free(&signal_pool, signal);
}

#endif

// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_signal_create(feabhOS_SIGNAL * const signal_handle)
{
  feabhOS_SIGNAL signal = allocate();
  if(signal == NULL) return ERROR_OUT_OF_MEMORY;

  OS_ERROR_TYPE OS_error;

  // Create a binary semaphore
  //
  OS_error = sem_init(&signal->handle, 0, 1);
  if(OS_error != 0) return ERROR_OUT_OF_MEMORY;
  
  // Put the semaphore in the 'taken' state.  If this
  // fails something very odd has happened.
  //
  sem_wait(&signal->handle);

  signal->waiting_tasks = 0;

  *signal_handle = signal;
  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_signal_notify_one(feabhOS_SIGNAL * const signal_handle)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(signal_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_SIGNAL signal = *signal_handle;

  // 'Give' the semaphore, releasing one waiting tasks
  //
  sem_post(&signal->handle);
  if(signal->waiting_tasks > 0)
  {
    signal->waiting_tasks--;
  }

  return ERROR_OK;
}

// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_signal_notify_all(feabhOS_SIGNAL * const signal_handle)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(signal_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_SIGNAL signal = *signal_handle;

  // 'Give' the semaphore, releasing all waiting tasks
  //
  while(signal->waiting_tasks > 0)
  {
    sem_post(&signal->handle);
    signal->waiting_tasks--;
  }

  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_signal_wait(feabhOS_SIGNAL * const signal_handle,
                                  duration_mSec_t        timeout)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(signal_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_SIGNAL signal = *signal_handle;

  OS_ERROR_TYPE OS_error;
  feabhOS_error error;

  // POSIX semphores don't support infinite timeouts, but
  // there are try-, blocking and timed-blocking calls
  //
  switch(timeout)
  {
  case NO_WAIT:
   OS_error = sem_trywait(&signal->handle);
   if (OS_error == 0) error = ERROR_OK;
   else               error = ERROR_TIMED_OUT;
   break;

  case WAIT_FOREVER:
   OS_error = sem_wait(&signal->handle);
   if (OS_error == 0) error = ERROR_OK;
   else               error = ERROR_UNKNOWN;
   break;

  default:
   {
     struct timespec abs_timeout = abs_duration(timeout);
     OS_error = sem_timedwait(&signal->handle, &abs_timeout);

     if(OS_error != 0)
     {
       switch(errno)
       {
       case EINVAL:
         error = ERROR_STUPID;
         break;
       case ETIMEDOUT:
         error = ERROR_TIMED_OUT;
         break;
       default:
         error = ERROR_UNKNOWN;
       }
     }

     if (OS_error == 0) error = ERROR_OK;
     else               error = ERROR_TIMED_OUT;
     break;
   }
  }

  return error;
}

// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_signal_destroy(feabhOS_SIGNAL * const signal_handle)
{
  // Parameter checking:
  //
  if(signal_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_SIGNAL signal = *signal_handle;

  sem_destroy(&signal->handle);
  deallocate(signal);

  return ERROR_OK;
}
