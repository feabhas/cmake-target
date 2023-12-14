// feabhOS_semaphore.c
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#include "feabhOS_semaphore.h"
#include <assert.h>
#include <stdbool.h>
#include <feabhOS_port_defs.h>

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

  semaphore->handle = xSemaphoreCreateCounting(max_count, init_count);
  if(semaphore->handle == NULL) return ERROR_OUT_OF_MEMORY;

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
  OS_ERROR_TYPE error;

  error = xSemaphoreTake(semaphore->handle, (OS_TIME_TYPE)timeout);

  if (error == pdPASS) return ERROR_OK;
  else                 return ERROR_TIMED_OUT;
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
  OS_ERROR_TYPE error = xSemaphoreGive(semaphore->handle);

  if (error == pdPASS) return ERROR_OK;
  else                 return ERROR_MAX_COUNT;
}

// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_semaphore_give_ISR(feabhOS_SEMAPHORE * const semaphore_handle)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(semaphore_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_SEMAPHORE semaphore = *semaphore_handle;
  BaseType_t wake_higher_priority = pdFALSE;

  OS_ERROR_TYPE error = xSemaphoreGiveFromISR(semaphore->handle, &wake_higher_priority);
  portYIELD_FROM_ISR(wake_higher_priority);

  if (error == pdPASS) return ERROR_OK;
  else                 return ERROR_MAX_COUNT;
}

// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_semaphore_destroy(feabhOS_SEMAPHORE * const semaphore_handle)
{
  // Parameter checking:
  //
  if(semaphore_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_SEMAPHORE semaphore = *semaphore_handle;

  deallocate(semaphore);

  // Ensure the client's handle is invalid
  //
  semaphore->handle = NULL;

  return ERROR_OK;
}

