// -----------------------------------------------------------------------------
// feabhOS_allocator.c
//
// DISCLAIMER:
// Feabhas is furnishing this item "as is". Feabhas does not provide any 
// warranty of the item whatsoever, whether express, implied, or statutory, 
// including, but not limited to, any warranty of merchantability or fitness
// for a particular purpose or any warranty that the contents of the item will 
// be error-free.
// In no respect shall Feabhas incur any liability for any damages, including, 
// but limited to, direct, indirect, special, or consequential damages arising
// out of, resulting from, or any way connected to the use of the item, whether 
// or not based upon warranty, contract, tort, or otherwise; whether or not 
// injury was sustained by persons or property or otherwise; and whether or not
// loss was sustained from, or arose out of, the results of, the item, or any 
// services that may be provided by Feabhas.
// -----------------------------------------------------------------------------

#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include "feabhOS_allocator.h"
#include "feabhOS_port_defs.h"
#include "feabhOS_time.h"

// ----------------------------------------------------------------------------
// Making a blocking call before the scheduler is started
// could lock up the entire system.  Therefore, any attempt
// to make a blocking call should assert.
//
extern bool scheduler_started;



typedef uintptr_t* block_ptr;

struct feabhOS_pool
{
  void*          start_addr;
  block_ptr      current_free;
  size_bytes_t   block_size;
  num_elements_t num_blocks;
};

// ----------------------------------------------------------------------------
//  Pool structure static allocation.
//  For simplicity, every time a pool is 'created'
//  by the client the next element in the array is
//  used; until they are all gone.
//  There is no re-use of pools in this implementation.
//
//  If all pools are taken the code will assert.
//
static feabhOS_POOL get_instance(void)
{
  static struct feabhOS_pool pools[MAX_POOLS];
  static unsigned int next_pool = 0;

  assert(next_pool != MAX_POOLS);

  return &pools[next_pool++];
}

// ----------------------------------------------------------------------------
// Static helper functions
//
static inline
block_ptr begin(feabhOS_POOL * const pool_handle)
{
  feabhOS_POOL pool = *pool_handle;
  return (block_ptr)pool->start_addr;
}


static inline
block_ptr end(feabhOS_POOL * const pool_handle)
{
  feabhOS_POOL pool = *pool_handle;
  return (block_ptr)(pool->start_addr + (pool->num_blocks * pool->block_size));
}


static inline
block_ptr next(feabhOS_POOL * const pool_handle, block_ptr ptr)
{
  feabhOS_POOL pool = *pool_handle;

  uint8_t *end_ptr = (uint8_t*)ptr + pool->block_size;

  return (block_ptr)end_ptr;
}


// ----------------------------------------------------------------------------
// Thread safety mechanism.
// The allocator can be used before the OS is started and while it is
// running.  If the OS is running there is a chance of race conditions
// so we must protect against it.
// Note, we cannot use a FeabhOS mutex to protect the allocator since
// FeabhOS mutexes themselves use the allocator!  Instead, we must
// use the underlying OS mutex mechanism.
//
static OS_MUTEX_TYPE alloc_lock = PTHREAD_MUTEX_INITIALIZER;

static inline
void init_lock(void)
{
  // if(mutex == NULL)
  // {
  //   pthread_mutex_init(&alloc_lock);
  // }
}

static inline
void lock(void)
{
  if(scheduler_started) pthread_mutex_lock(&alloc_lock);
}

static inline
void unlock(void)
{
  if(scheduler_started) pthread_mutex_unlock(&alloc_lock);
}

// The EXIT_WITH macro ensure that the mutex
// is unlocked before returning.
//
#define LOCK                  lock
#define UNLOCK                unlock
#define EXIT_WITH(return_val) { unlock(); return (return_val); }


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_pool_create(feabhOS_POOL * const pool_handle,
                                  void*                pool_memory,
                                  size_bytes_t         pool_size,
                                  size_bytes_t         block_size,
                                  num_elements_t       num_blocks)
{
  // First call - initialise the lock(mutex).  This call will
  // be thread-safe since the first calls to this function
  // will be before the scheduler is started (you must have
  // at least one task!)
  //
  init_lock();

  LOCK();

  // Parameter checking
  //
  if(pool_memory == NULL)                   EXIT_WITH(ERROR_PARAM1);
  if(pool_size == 0)                        EXIT_WITH(ERROR_PARAM2);
  if(block_size == 0)                       EXIT_WITH(ERROR_PARAM3);
  if(block_size < sizeof(uintptr_t))        EXIT_WITH(ERROR_PARAM3);
  if(num_blocks == 0)                       EXIT_WITH(ERROR_PARAM4);
  if((block_size * num_blocks) > pool_size) EXIT_WITH(ERROR_PARAM2);


  feabhOS_POOL pool = get_instance();

  pool->start_addr   = pool_memory;
  pool->current_free = pool_memory;
  pool->block_size   = block_size;
  pool->num_blocks   = num_blocks;

  // Initialise the pool by creating the 'free list'
  // at the beginning of each block.
  //
  block_ptr iter = pool->current_free;

  for(unsigned int i = 0; i < (num_blocks - 1); ++i)
  {
    *iter = (uintptr_t)next(&pool, iter);
    iter = next(&pool, iter);
  }

  // Terminate the free-list
  //
  *((uintptr_t*)iter) = (uintptr_t)NULL;

  *pool_handle = pool;

  EXIT_WITH(ERROR_OK);
}


// ----------------------------------------------------------------------------
//
void* feabhOS_block_allocate(feabhOS_POOL * const pool_handle)
{
  LOCK();

  // Parameter checking
  //
  if(pool_handle == NULL) EXIT_WITH(NULL);

  feabhOS_POOL pool = *pool_handle;

  // Allocate the next free block.  If the current_free pointer
  // is NULL it means we've run out of memory.
  //
  if(pool->current_free == NULL) EXIT_WITH(NULL);

  // The current_free pointer holds the address of the next
  // block to allocate.  At that location is stored the address
  // of the next free block.
  //
  void *user_data = pool->current_free;
  pool->current_free = (uintptr_t*)*(pool->current_free);


  EXIT_WITH(user_data);
}

// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_block_free(feabhOS_POOL * const pool_handle, void * block)
{
  LOCK();

  // Parameter checking
  //
  if(pool_handle == NULL)                          EXIT_WITH(ERROR_INVALID_HANDLE);
  if(block == NULL)                                EXIT_WITH(ERROR_OK);
  if(((uintptr_t*)block <  begin(pool_handle)) ||
     ((uintptr_t*)block >= end(pool_handle)))      EXIT_WITH(ERROR_STUPID);


  feabhOS_POOL pool = *pool_handle;

  // Return the block to the head of the free list.
  //
  block_ptr to_free = (block_ptr)block;

  *to_free = (uintptr_t)pool->current_free;
  pool->current_free = to_free;

  EXIT_WITH(ERROR_OK);
}
