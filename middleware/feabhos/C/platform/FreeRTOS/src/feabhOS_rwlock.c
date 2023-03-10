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
#include "feabhOS_rwlock.h"
#include "feabhOS_port_defs.h"
#include "feabhOS_mutex.h"
#include "feabhOS_condition.h"
#include "feabhOS_task.h"

// ----------------------------------------------------------------------------
// Making a blocking call before the scheduler is started
// could lock up the entire system.  Therefore, any attempt
// to make a blocking call should assert.
//
extern bool scheduler_started;

// ----------------------------------------------------------------------------
// Management structure
//
struct feabhOS_rwlock
{
  feabhOS_MUTEX     lock;
  feabhOS_CONDITION write_available;
  feabhOS_CONDITION read_available;
  unsigned int      active_readers;
  unsigned int      waiting_writers;
  bool              is_writer_active;
};


// ----------------------------------------------------------------------------
//
//  MEMORY MANAGEMENT FOR RW-LOCK STRUCTURES
//  ---------------------------------------
//
//  For a fixed number of rw-locks we use a fixed-block
//  dynamic allocator.
//  If MAX_TASKS == NO_LIMIT we use the underlying OS'
//  dynamic memory allocator (usually malloc)
//

#if MAX_RWLOCKS==NO_LIMIT

#include "feabhOS_memory.h"

static feabhOS_RWLOCK allocate(void)
{
  return feabhOS_memory_alloc(sizeof(struct feabhOS_rwlock));
}


static void deallocate(feabhOS_RWLOCK rwlock)
{
  feabhOS_memory_free(rwlock);
}

#else

#include "feabhOS_allocator.h"

static struct feabhOS_rwlock rwlocks[MAX_RWLOCKS];
static feabhOS_POOL rwlock_pool = NULL;


static feabhOS_RWLOCK allocate(void)
{
  // Craete the pool first time a task is allocated
  //
  if(rwlock_pool == NULL)
  {
    feabhOS_pool_create(&rwlock_pool,
                        rwlocks,
                        sizeof(rwlocks),
                        sizeof(struct feabhOS_rwlock),
                        MAX_RWLOCKS);
  }

  return feabhOS_block_allocate(&rwlock_pool);
}


static void deallocate(feabhOS_RWLOCK rwlock)
{
  feabhOS_block_free(&rwlock_pool, rwlock);
}

#endif

// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_rwlock_create(feabhOS_RWLOCK * const rwlock_handle)
{
  feabhOS_RWLOCK rwlock = allocate();
  if(rwlock == NULL) return ERROR_OUT_OF_MEMORY;

  feabhOS_error error;

  error = feabhOS_mutex_create(&rwlock->lock);
  if(error != ERROR_OK) return error;

  error = feabhOS_condition_create(&rwlock->read_available);
  if(error != ERROR_OK) return error;

  error = feabhOS_condition_create(&rwlock->write_available);
  if(error != ERROR_OK) return error;

  rwlock->active_readers   = 0;
  rwlock->waiting_writers  = 0;
  rwlock->is_writer_active = false;

  *rwlock_handle = rwlock;
  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_rwlock_read_acquire(feabhOS_RWLOCK * const rwlock_handle)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(rwlock_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_RWLOCK rwlock = *rwlock_handle;

  feabhOS_mutex_lock(&rwlock->lock, WAIT_FOREVER);

  // 1.  If only readers, just increment the number of readers.
  // 2.  If a thread is currently writing, wait until it's finished.
  // 3.  If there are any waiting writers, wait until ALL waiting
  //     writers have finished (otherwise the writers may starve).
  //
  while(rwlock->is_writer_active || (rwlock->waiting_writers > 0))
  {
    feabhOS_condition_wait(&rwlock->read_available, &rwlock->lock, WAIT_FOREVER);
  }
  rwlock->active_readers++;

  feabhOS_mutex_unlock(&rwlock->lock);

  // If there are a set of waiting readers we cannot guarantee
  // (because of OS behaviour) that they will all be released.
  // Therefore, force a re-schedule (via a yield) to allow all
  // equal-prioritied threads a chance to read.
  //
  feabhOS_task_yield();
  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_rwlock_read_release(feabhOS_RWLOCK * const rwlock_handle)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(rwlock_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_RWLOCK rwlock = *rwlock_handle;

  feabhOS_mutex_lock(&rwlock->lock, WAIT_FOREVER);

  // 1.  If this thread is the last reader and there are waiting
  //     writers, wake up ONE writer.
  //
  rwlock->active_readers--;

  if((rwlock->active_readers == 0) && (rwlock->waiting_writers > 0))
  {
    feabhOS_condition_notify_one(&rwlock->write_available);
  }

  feabhOS_mutex_unlock(&rwlock->lock);
  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_rwlock_write_acquire(feabhOS_RWLOCK * const rwlock_handle)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(rwlock_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_RWLOCK rwlock = *rwlock_handle;

  feabhOS_mutex_lock(&rwlock->lock, WAIT_FOREVER);

  // 1.  While there are threads actively reading, tell them
  //     this thread is waiting to write, but then block until
  //     the LAST reader has left.
  // 2.  As there are no more readers, if the current thread
  //     is woken it MUST be the active writer.
  //
  while(rwlock->active_readers > 0)
  {
    rwlock->waiting_writers++;
    feabhOS_condition_wait(&rwlock->write_available, &rwlock->lock, WAIT_FOREVER);
    rwlock->waiting_writers--;
  }
  rwlock->is_writer_active = true;

  feabhOS_mutex_unlock(&rwlock->lock);
  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_rwlock_write_release(feabhOS_RWLOCK * const rwlock_handle)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(rwlock_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_RWLOCK rwlock = *rwlock_handle;

  feabhOS_mutex_lock(&rwlock->lock, WAIT_FOREVER);

  // 1.  The current thread MUST be a writer (and has finished).
  // 2.  If there are waiting writers wake the next one up.
  //     Otherwise, wake up ALL the readers.
  //
  rwlock->is_writer_active = false;
  if(rwlock->waiting_writers > 0)
  {
    feabhOS_condition_notify_one(&rwlock->write_available);
  }
  else
  {
    feabhOS_condition_notify_all(&rwlock->read_available);
  }

  feabhOS_mutex_unlock(&rwlock->lock);
  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_rwlock_destroy(feabhOS_RWLOCK * const rwlock_handle)
{
  // Parameter checking:
  //
  if(rwlock_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_RWLOCK rwlock = *rwlock_handle;

  feabhOS_mutex_destroy(&rwlock->lock);
  feabhOS_condition_destroy(&rwlock->read_available);
  feabhOS_condition_destroy(&rwlock->write_available);

  deallocate(rwlock);

  rwlock->lock            = NULL;
  rwlock->read_available  = NULL;
  rwlock->write_available = NULL;

  return ERROR_OK;
}
