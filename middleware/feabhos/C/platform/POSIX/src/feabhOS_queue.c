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
#include <string.h>
#include "feabhOS_queue.h"
#include "feabhOS_mutex.h"
#include "feabhOS_condition.h"
#include "feabhOS_port_defs.h"
#include "feabhOS_memory.h"


// ----------------------------------------------------------------------------
// POSIX queues are designed for inter-process communication
// and too heavyweight for a simple, single-process framework
// like FeabhOS.
// Therefore, we'll implement the FeabhOS queue as a simple,
// fixed-size FIFO.
//
// NOTE:
// For simplicity, the memory for the queue data will be
// dynamically allocated (using feabhOS_memory_alloc()); even
// if the memory for the queue management structure is not.
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
struct feabhOS_queue
{
  void*             buffer;
  size_bytes_t      element_size;
  num_elements_t    capacity;
  num_elements_t    num_items;
  unsigned int      read;
  unsigned int      write;
  feabhOS_MUTEX     mutex;
  feabhOS_CONDITION has_data;
  feabhOS_CONDITION has_space;
};

// ----------------------------------------------------------------------------
//
//  MEMORY MANAGEMENT FOR QUEUE STRUCTURES
//  ---------------------------------------
//
//  For a fixed number of queues we use a fixed-block
//  dynamic allocator.
//  If MAX_QUEUES == NO_LIMIT we use the underlying OS'
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
  // Parameter checking
  //
  if(elem_size == 0)  return ERROR_PARAM1;
  if(queue_size == 0) return ERROR_PARAM2;

  feabhOS_QUEUE queue = allocate();
  if(queue == NULL) return ERROR_OUT_OF_MEMORY;

  queue->buffer      = feabhOS_memory_alloc(elem_size * queue_size);
  queue->element_size = elem_size;
  queue->capacity     = queue_size;
  queue->read         = 0;
  queue->write        = 0;
  queue->num_items    = 0;

  if(queue->buffer == NULL)                                   return ERROR_OUT_OF_MEMORY;
  if(feabhOS_mutex_create(&queue->mutex)         != ERROR_OK) return ERROR_OUT_OF_MEMORY;
  if(feabhOS_condition_create(&queue->has_data)  != ERROR_OK) return ERROR_OUT_OF_MEMORY;
  if(feabhOS_condition_create(&queue->has_space) != ERROR_OK) return ERROR_OUT_OF_MEMORY;
  
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

  feabhOS_mutex_lock(&queue->mutex, WAIT_FOREVER);

  // Block while the queue is full; or until timeout
  //
  while(queue->num_items == queue->capacity)
  {
    feabhOS_error error;
    error = feabhOS_condition_wait(&queue->has_space, &queue->mutex, timeout);
    if(error != ERROR_OK)
    {
      feabhOS_mutex_unlock(&queue->mutex);
      return ERROR_TIMED_OUT;
    }
  }

  // Copy the item to the next location in the buffer,
  // wrapping around as necessary.
  //
  uint8_t *write_location = (uint8_t *)queue->buffer + (queue->write * queue->element_size);
  memcpy(write_location, in, queue->element_size);
  queue->num_items++;
  queue->write++;
  if(queue->write == queue->capacity) queue->write = 0;

  // Signal any waiting tasks that there
  // is now some data in the buffer
  //
  feabhOS_condition_notify_all(&queue->has_data);

  feabhOS_mutex_unlock(&queue->mutex);
  return ERROR_OK;
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
  
  feabhOS_mutex_lock(&queue->mutex, WAIT_FOREVER);

  // Block while the queue is empty; or until
  // the timeout expires
  //
  while (queue->num_items == 0)
  {
    feabhOS_error error;
    error = feabhOS_condition_wait(&queue->has_data, &queue->mutex, timeout);
    if(error != ERROR_OK)
    {
      feabhOS_mutex_unlock(&queue->mutex);
      return ERROR_TIMED_OUT;
    }
  }

  // Copy the item from the buffer
  //
  uint8_t *read_location = (uint8_t *)queue->buffer + (queue->read * queue->element_size);
  memcpy(in_out, read_location, queue->element_size);
  queue->num_items--;
  queue->read++;
  if(queue->read == queue->capacity) queue->read = 0;

  // Signal any waiting tasks that there
  // is now some space in the buffer
  //
  feabhOS_condition_notify_all(&queue->has_space);

  feabhOS_mutex_unlock(&queue->mutex);
  return ERROR_OK;
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

  num_elements_t size;
  feabhOS_mutex_lock(&queue->mutex, WAIT_FOREVER);
  size = queue->num_items;
  feabhOS_mutex_unlock(&queue->mutex);
  return size;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_queue_destroy(feabhOS_QUEUE *queue_handle)
{
  // Parameter checking:
  //
  if(queue_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_QUEUE queue = *queue_handle;

  //  Make sure we clean up all the component parts
  //  of the queue.
  //
  feabhOS_memory_free      (queue->buffer);
  feabhOS_mutex_destroy    (&queue->mutex);
  feabhOS_condition_destroy(&queue->has_data);
  feabhOS_condition_destroy(&queue->has_space);

  deallocate(queue);
  return ERROR_OK;
}

