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
#include "feabhOS_mutex.h"
#include "feabhOS_port_defs.h"

// ----------------------------------------------------------------------------
// Making a blocking call before the scheduler is started
// could lock up the entire system.  Therefore, any attempt
// to make a blocking call should assert.
//
extern bool scheduler_started;

// ----------------------------------------------------------------------------
// Management structure
//
struct feabhOS_mutex
{
  OS_MUTEX_TYPE handle;
};

// ----------------------------------------------------------------------------
//
//  MEMORY MANAGEMENT FOR MUTEX STRUCTURES
//  ---------------------------------------
//
//  For a fixed number of mutexes we use a fixed-block
//  dynamic allocator.
//  If MAX_TASKS == NO_LIMIT we use the underlying OS'
//  dynamic memory allocator (usually malloc)
//

#if MAX_MUTEXES==NO_LIMIT

#include "feabhOS_memory.h"

static feabhOS_MUTEX allocate(void)
{
  return feabhOS_memory_alloc(sizeof(struct feabhOS_mutex));
}


static void deallocate(feabhOS_MUTEX mutex)
{
  feabhOS_memory_free(mutex);
}

#else

#include "feabhOS_allocator.h"

static struct feabhOS_mutex mutexes[MAX_MUTEXES];
static feabhOS_POOL mutex_pool = NULL;


static feabhOS_MUTEX allocate(void)
{
  // Craete the pool first time a task is allocated
  //
  if(mutex_pool == NULL)
  {
    feabhOS_pool_create(&mutex_pool,
                        mutexes,
                        sizeof(mutexes),
                        sizeof(struct feabhOS_mutex),
                        MAX_MUTEXES);
  }

  return feabhOS_block_allocate(&mutex_pool);
}


static void deallocate(feabhOS_MUTEX mutex)
{
  feabhOS_block_free(&mutex_pool, mutex);
}

#endif

// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_mutex_create(feabhOS_MUTEX * const mutex_handle)
{
  feabhOS_MUTEX mutex = allocate();
  if(mutex == NULL) return ERROR_OUT_OF_MEMORY;

  mutex->handle = xSemaphoreCreateMutex();
  if(mutex->handle == 0) return ERROR_OUT_OF_MEMORY;
  
  *mutex_handle = mutex;
  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_mutex_lock(feabhOS_MUTEX * const mutex_handle, duration_mSec_t timeout)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(mutex_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_MUTEX mutex = *mutex_handle;
  OS_ERROR_TYPE OSError;

  OSError = xSemaphoreTake(mutex->handle, (OS_TIME_TYPE)timeout);

  if (OSError == pdPASS) return ERROR_OK;
  else                   return ERROR_TIMED_OUT;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_mutex_unlock(feabhOS_MUTEX * const mutex_handle)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(mutex_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_MUTEX mutex = *mutex_handle;
  OS_ERROR_TYPE OSError;

  OSError = xSemaphoreGive(mutex->handle);

  if (OSError == pdPASS) return ERROR_OK;
  else                   return ERROR_NOT_OWNER;

  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_mutex_destroy(feabhOS_MUTEX * const mutex_handle)
{
  // Parameter checking:
  //
  if(mutex_handle == NULL) return ERROR_INVALID_HANDLE;

  // Ensure client's handle is invalid
  //
  feabhOS_MUTEX mutex = *mutex_handle;
  deallocate(mutex);
  mutex->handle = NULL;

  return ERROR_OK;
}
