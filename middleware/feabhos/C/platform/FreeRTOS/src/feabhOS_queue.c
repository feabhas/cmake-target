// feabhOS_queue.c
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#include <assert.h>
#include <stdbool.h>
#include "feabhOS_queue.h"
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
struct feabhOS_queue
{
  OS_QUEUE_TYPE handle;
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

#if MAX_QUEUES==NO_LIMIT

#include "feabhOS_memory.h"

static feabhOS_QUEUE allocate(void)
{
  return feabhOS_memory_alloc(sizeof(struct feabhOS_queue));
}


static void deallocate(feabhOS_QUEUE queue)
{
  feabhOS_memory_free(queue);
}

#else

#include "feabhOS_allocator.h"

static struct feabhOS_queue queues[MAX_QUEUES];
static feabhOS_POOL queue_pool = NULL;


static feabhOS_QUEUE allocate(void)
{
  // Craete the pool first time a task is allocated
  //
  if(queue_pool == NULL)
  {
    feabhOS_pool_create(&queue_pool,
                        queues,
                        sizeof(queues),
                        sizeof(struct feabhOS_queue),
                        MAX_QUEUES);
  }

  return feabhOS_block_allocate(&queue_pool);
}


static void deallocate(feabhOS_QUEUE queue)
{
  feabhOS_block_free(&queue_pool, queue);
}

#endif

// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_queue_create(feabhOS_QUEUE * const queue_handle,
                                   size_bytes_t          elem_size,
                                   num_elements_t        queue_size)
{
  feabhOS_QUEUE queue = allocate();
  if(queue == NULL) return ERROR_OUT_OF_MEMORY;

  queue->handle = xQueueCreate((OS_UNSIGNED_TYPE)queue_size, (OS_UNSIGNED_TYPE)elem_size);
  if(queue->handle == 0) return ERROR_OUT_OF_MEMORY;

  *queue_handle = queue;
  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_queue_post(feabhOS_QUEUE * const queue_handle,
                                 void          * const in,
                                 duration_mSec_t       timeout)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(queue_handle == NULL) return ERROR_INVALID_HANDLE;
  if(in == NULL)           return ERROR_PARAM1;

  feabhOS_QUEUE queue = *queue_handle;

  OS_ERROR_TYPE OSError = xQueueSendToBack(queue->handle, in, (OS_TIME_TYPE)timeout);

  if(OSError == pdPASS) return ERROR_OK;
  else                  return ERROR_QUEUE_FULL;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_queue_get(feabhOS_QUEUE * const queue_handle,
                                void          * const in_out,
                                duration_mSec_t       timeout)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(queue_handle == NULL) return ERROR_INVALID_HANDLE;
  if(in_out == NULL)          return ERROR_PARAM1;

  feabhOS_QUEUE queue = *queue_handle;

  OS_ERROR_TYPE OSError = xQueueReceive(queue->handle, in_out, (OS_TIME_TYPE)timeout);

  if(OSError == pdPASS) return ERROR_OK;
  else                  return ERROR_QUEUE_EMPTY;
}


// ----------------------------------------------------------------------------
//
num_elements_t feabhOS_queue_size(feabhOS_QUEUE *queue_handle)
{
  // Parameter checking:
  // An invalid queue will always be empty!
  //
  assert(scheduler_started == true);
  if(queue_handle == NULL) return 0;

  feabhOS_QUEUE queue = *queue_handle;
  return (num_elements_t)uxQueueMessagesWaiting(queue->handle);
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_queue_destroy(feabhOS_QUEUE *queue_handle)
{
  // Parameter checking:
  //
  if(queue_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_QUEUE queue = *queue_handle;
  vQueueDelete(queue->handle);

  deallocate(queue);

  // Ensure the client's handle is invalid
  //
  queue->handle = NULL;

  return ERROR_OK;
}

