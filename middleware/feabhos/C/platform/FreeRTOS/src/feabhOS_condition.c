// feabhOS_condition.c
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#include <stdbool.h>
#include "feabhOS_condition.h"
#include "feabhOS_signal.h"
#include "feabhOS_mutex.h"
#include <assert.h>
#include <feabhOS_port_defs.h>

// ----------------------------------------------------------------------------
// Making a blocking call before the scheduler is started
// could lock up the entire system.  Therefore, any attempt
// to make a blocking call should assert.
//
extern bool scheduler_started;

// ----------------------------------------------------------------------------
// Management structure
//
struct feabhOS_condition
{
  feabhOS_SIGNAL signal;

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

#if MAX_CONDITIONS==NO_LIMIT

#include "feabhOS_memory.h"

static feabhOS_CONDITION allocate(void)
{
  return feabhOS_memory_alloc(sizeof(struct feabhOS_condition));
}


static void deallocate(feabhOS_CONDITION condition)
{
  feabhOS_memory_free(condition);
}

#else

#include "feabhOS_allocator.h"

static struct feabhOS_condition conditions[MAX_CONDITIONS];
static feabhOS_POOL condition_pool = NULL;


static feabhOS_CONDITION allocate(void)
{
  // Craete the pool first time a task is allocated
  //
  if(condition_pool == NULL)
  {
    feabhOS_pool_create(&condition_pool,
                        conditions,
                        sizeof(conditions),
                        sizeof(struct feabhOS_condition),
                        MAX_CONDITIONS);
  }

  return feabhOS_block_allocate(&condition_pool);
}


static void deallocate(feabhOS_CONDITION condition)
{
  feabhOS_block_free(&condition_pool, condition);
}

#endif

// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_condition_create(feabhOS_CONDITION * const condition_handle)
{
  feabhOS_CONDITION condition;
  feabhOS_error error;

  condition = allocate();
  if(condition == NULL) return ERROR_OUT_OF_MEMORY;

  error = feabhOS_signal_create(&condition->signal);
  if(error == ERROR_OUT_OF_MEMORY) return error;

  *condition_handle = condition;

  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_condition_notify_one(feabhOS_CONDITION * const condition_handle)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(condition_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_CONDITION condition = *condition_handle;
  feabhOS_signal_notify_one(&condition->signal);

  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_condition_notify_all(feabhOS_CONDITION * const condition_handle)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(condition_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_CONDITION condition = *condition_handle;
  feabhOS_signal_notify_all(&condition->signal);

  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_condition_wait(feabhOS_CONDITION * const condition_handle,
                                     feabhOS_MUTEX     * const mutex_handle,
                                     duration_mSec_t           timeout)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(condition_handle == NULL) return ERROR_INVALID_HANDLE;
  if(mutex_handle == NULL)     return ERROR_PARAM1;

  feabhOS_CONDITION condition = *condition_handle;
  feabhOS_error err;

  feabhOS_mutex_unlock(mutex_handle);
  err = feabhOS_signal_wait(&condition->signal, timeout);
  feabhOS_mutex_lock(mutex_handle, WAIT_FOREVER);

  return err;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_condition_destroy(feabhOS_CONDITION * const condition_handle)
{
  // Parameter checking:
  //
  if(condition_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_CONDITION condition = *condition_handle;

  feabhOS_signal_destroy(&condition->signal);
  deallocate(condition);

  // Ensure client's handle is invalid
  //
  condition->signal = NULL;

  return ERROR_OK;
}
