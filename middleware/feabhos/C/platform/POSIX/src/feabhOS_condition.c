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
#include <pthread.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include "feabhOS_condition.h"
#include "feabhOS_mutex.h"
#include "feabhOS_port_defs.h"
#include "feabhOS_time_utils.h"

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
  OS_CONDITION_TYPE handle;
  
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
  OS_ERROR_TYPE OS_error;

  condition = allocate();
  if(condition == NULL) return ERROR_OUT_OF_MEMORY;

  OS_error = pthread_cond_init(&condition->handle, NULL);

  if(OS_error != 0) return ERROR_OUT_OF_MEMORY;
  
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
  OS_ERROR_TYPE OS_error = pthread_cond_signal(&condition->handle);

  if(OS_error == 0) return ERROR_OK;
  else              return ERROR_UNKNOWN;
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
  OS_ERROR_TYPE OS_error = pthread_cond_broadcast(&condition->handle);

  if(OS_error == 0) return ERROR_OK;
  else              return ERROR_UNKNOWN;
}


// ----------------------------------------------------------------------------
// The POSIX implementation of condition variables requires access
// to the POSIX definition of what a mutex is.  Here, we are using
// a (hidden) accessor function to retrieve this information
//
extern OS_MUTEX_TYPE* feabhOS_mutex_native_handle(feabhOS_MUTEX * const mutex_handle);


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
  OS_ERROR_TYPE OS_error;
  feabhOS_error error;

  OS_MUTEX_TYPE *mutex = feabhOS_mutex_native_handle(mutex_handle);
  
  // POSiX doesn't support infinite timeouts, but it does have
  // blocking and timed-blocking calls
  //
  if(timeout == WAIT_FOREVER)
  {
    OS_error = pthread_cond_wait(&condition->handle, mutex);
    if(OS_error == 0) error = ERROR_OK;
    else              error = ERROR_UNKNOWN;
  }
  else
  {
    struct timespec abs_timeout = abs_duration(timeout);
    OS_error = pthread_cond_timedwait(&condition->handle, mutex, &abs_timeout);

    switch(OS_error)
    {
    case ETIMEDOUT:
      error = ERROR_TIMED_OUT;
      break;

    case EINVAL:
      error = ERROR_STUPID;
      break;

    default:
      error = ERROR_OK;
      break;
    }
  }

  return error;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_condition_destroy(feabhOS_CONDITION * const condition_handle)
{
  // Parameter checking:
  //
  if(condition_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_CONDITION condition = *condition_handle;
  OS_ERROR_TYPE OS_error;

  OS_error = pthread_cond_destroy(&condition->handle);
  if(OS_error != 0) return ERROR_STUPID;

  deallocate(condition);

  return ERROR_OK;
}
