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
#include "feabhOS_semaphore.h"
#include "feabhOS_port_defs.h"
#include "feabhOS_time_utils.h"

/// ----------------------------------------------------------------------------
// Making a blocking call before the scheduler is started
// could lock up the entire system.  Therefore, any attempt
// to make a blocking call should assert.
//
extern bool scheduler_started;

// ----------------------------------------------------------------------------
// Management structure
//
struct feabhOS_semaphore
{
  OS_COUNTING_SEMAPHORE_TYPE handle;
  num_elements_t             max;
};

// ----------------------------------------------------------------------------
//
//  MEMORY MANAGEMENT FOR SEMAPHORE STRUCTURES
//  ------------------------------------------
//
//  For a fixed number of semaphores we use a fixed-block
//  dynamic allocator.
//  If MAX_TASKS == NO_LIMIT we use the underlying OS'
//  dynamic memory allocator (usually malloc)
//

#if MAX_SEMAPHORES==NO_LIMIT

#include "feabhOS_memory.h"

static feabhOS_SEMAPHORE allocate()
{
  return feabhOS_memory_alloc(sizeof(struct feabhOS_semaphore));
}


static void deallocate(feabhOS_SEMAPHORE semaphore)
{
  feabhOS_memory_free(semaphore);
}

#else

#include "feabhOS_allocator.h"

static struct feabhOS_semaphore semaphores[MAX_SEMAPHORES];
static feabhOS_POOL semaphore_pool = NULL;


static feabhOS_SEMAPHORE allocate()
{
  // Craete the pool first time a task is allocated
  //
  if(semaphore_pool == NULL)
  {
    feabhOS_pool_create(&semaphore_pool,
                        semaphores,
                        sizeof(semaphores),
                        sizeof(struct feabhOS_semaphore),
                        MAX_SEMAPHORES);
  }

  return feabhOS_block_allocate(&semaphore_pool);
}


static void deallocate(feabhOS_SEMAPHORE semaphore)
{
  feabhOS_block_free(&semaphore_pool, semaphore);
}

#endif


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_semaphore_create(feabhOS_SEMAPHORE * const semaphore_handle,
                                       num_elements_t            max_count,
                                       num_elements_t            init_count)
{
  feabhOS_SEMAPHORE semaphore;

  semaphore = allocate();
  if(semaphore == NULL) return ERROR_OUT_OF_MEMORY;

  semaphore->max = max_count;

  OS_ERROR_TYPE OS_error;

  OS_error = sem_init(&semaphore->handle, 0, init_count);
  if(OS_error != 0) return ERROR_OUT_OF_MEMORY;
  
  *semaphore_handle = semaphore;
  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_semaphore_take(feabhOS_SEMAPHORE * const semaphore_handle,
                                     duration_mSec_t           timeout)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(semaphore_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_SEMAPHORE semaphore = *semaphore_handle;
  OS_ERROR_TYPE OS_error;
  feabhOS_error error;

  // POSIX semphores don't support infinite timeouts, but
  // there are try-, blocking and timed-blocking calls
  //
  switch(timeout)
  {
  case NO_WAIT:
   OS_error = sem_trywait(&semaphore->handle);
   if (OS_error == 0) error = ERROR_OK;
   else               error = ERROR_TIMED_OUT;
   break;

  case WAIT_FOREVER:
   OS_error = sem_wait(&semaphore->handle);
   if (OS_error == 0) error = ERROR_OK;
   else               error = ERROR_UNKNOWN;
   break;

  default:
   {
     struct timespec abs_timeout = abs_duration(timeout);
     OS_error = sem_timedwait(&semaphore->handle, &abs_timeout);
     if (OS_error == 0) error = ERROR_OK;
     else               error = ERROR_TIMED_OUT;
     break;
   }
  }
  
  return error;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_semaphore_give(feabhOS_SEMAPHORE * const semaphore_handle)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(semaphore_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_SEMAPHORE semaphore = *semaphore_handle;

  num_elements_t current_count;
  sem_getvalue(&semaphore->handle, &current_count);

  if((num_elements_t)current_count == semaphore->max)
  {
    return ERROR_MAX_COUNT;
  }
  else
  {
    sem_post(&semaphore->handle);
    return ERROR_OK;
  }
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_semaphore_destroy(feabhOS_SEMAPHORE * const semaphore_handle)
{
  // Parameter checking:
  //
  if(semaphore_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_SEMAPHORE semaphore = *semaphore_handle;

  sem_destroy(&semaphore->handle);
  deallocate(semaphore);

  return ERROR_OK;
}

