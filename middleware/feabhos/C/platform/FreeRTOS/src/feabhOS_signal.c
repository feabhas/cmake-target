// feabhOS_signal.c
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#include <assert.h>
#include <stdbool.h>
#include "feabhOS_signal.h"
#include <feabhOS_port_defs.h>


// ----------------------------------------------------------------------------
//  NOTE:
//  FreeRTOS does not support the idea of a
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

  vSemaphoreCreateBinary(signal->handle);
  if(signal->handle == NULL) return ERROR_OUT_OF_MEMORY;

  // Put the semaphore in the 'taken' state.  If this
  // fails something very odd has happened.
  //
  xSemaphoreTake(signal->handle, WAIT_FOREVER);

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

  // Give the semaphore, releasing any waiting tasks
  //
  xSemaphoreGive(signal->handle);
  if(signal->waiting_tasks > 0)
  {
    signal->waiting_tasks--;
  }

  return ERROR_OK;
}

// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_signal_notify_one_ISR(feabhOS_SIGNAL * const signal_handle)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(signal_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_SIGNAL signal = *signal_handle;
  BaseType_t wake_higher_priority = pdFALSE;

  // Give the semaphore, releasing any waiting tasks
  //
  xSemaphoreGiveFromISR(signal->handle, &wake_higher_priority);
  if(signal->waiting_tasks > 0)
  {
    signal->waiting_tasks--;
  }
  portYIELD_FROM_ISR(wake_higher_priority);

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

  /* Give the semaphore, releasing any waiting tasks */
  while(signal->waiting_tasks > 0)
  {
    xSemaphoreGive(signal->handle);
    signal->waiting_tasks--;
  }

  return ERROR_OK;
}

// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_signal_notify_all_ISR(feabhOS_SIGNAL * const signal_handle)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(signal_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_SIGNAL signal = *signal_handle;
  BaseType_t wake_higher_priority = pdFALSE;

  /* Give the semaphore, releasing any waiting tasks */
  while(signal->waiting_tasks > 0)
  {
    BaseType_t wake_next = pdFALSE;
    xSemaphoreGiveFromISR(signal->handle, &wake_next);
    if (wake_higher_priority == pdFALSE && wake_next != pdFALSE) {
        wake_higher_priority = wake_next;
    }
    signal->waiting_tasks--;
  }
  portYIELD_FROM_ISR(wake_higher_priority);

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

  // Attempt to take the semaphore, which should pend
  //
  OS_ERROR_TYPE error;

  signal->waiting_tasks++;
  error = xSemaphoreTake(signal->handle, timeout);

  if(error == pdPASS) return ERROR_OK;
  else                return ERROR_TIMED_OUT;
}

// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_signal_destroy(feabhOS_SIGNAL * const signal_handle)
{
  // Parameter checking:
  //
  if(signal_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_SIGNAL signal = *signal_handle;

  deallocate(signal);

  // Ensure the client's handle is invalid
  //
  signal->handle = NULL;
  signal->waiting_tasks = 0;

  return ERROR_OK;
}
