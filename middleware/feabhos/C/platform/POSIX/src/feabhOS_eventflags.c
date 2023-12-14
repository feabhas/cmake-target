// feabhOS_eventflags.c
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#include <stdbool.h>
#include <assert.h>
#include "feabhOS_eventflags.h"
#include "feabhOS_mutex.h"
#include "feabhOS_condition.h"
#include "feabhOS_port_defs.h"


// ----------------------------------------------------------------------------
// POSIX doesn't support event flags so we will have to simulate blocking
// on event flag groups using the Guarded Suspension pattern.
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
struct feabhOS_eventflags
{
  bitmask8_t        flags;
  feabhOS_MUTEX     mutex;
  feabhOS_CONDITION flag_set;
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

#if MAX_EVENTFLAGS==NO_LIMIT

#include "feabhOS_memory.h"

static feabhOS_EVENTFLAGS allocate(void)
{
  return feabhOS_memory_alloc(sizeof(struct feabhOS_eventflags));
}


static void deallocate(feabhOS_EVENTFLAGS flags)
{
  feabhOS_memory_free(flags);
}

#else

#include "feabhOS_allocator.h"

static struct feabhOS_eventflags event_flags[MAX_EVENTFLAGS];
static feabhOS_POOL eventflags_pool = NULL;


static feabhOS_EVENTFLAGS allocate(void)
{
  // Craete the pool first time a task is allocated
  //
  if(eventflags_pool == NULL)
  {
    feabhOS_pool_create(&eventflags_pool,
                        event_flags,
                        sizeof(event_flags),
                        sizeof(struct feabhOS_eventflags),
                        MAX_EVENTFLAGS);
  }

  return feabhOS_block_allocate(&eventflags_pool);
}


static void deallocate(feabhOS_EVENTFLAGS flags)
{
  feabhOS_block_free(&eventflags_pool, flags);
}

#endif

// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_eventflags_create(feabhOS_EVENTFLAGS * const events_handle)
{
  feabhOS_EVENTFLAGS event_flags = allocate();
  if(event_flags == NULL) return ERROR_OUT_OF_MEMORY;

  event_flags->flags = 0;

  feabhOS_error error;

  error = feabhOS_mutex_create(&event_flags->mutex);
  if(error != ERROR_OK) return error;

  error = feabhOS_condition_create(&event_flags->flag_set);
  if(error != ERROR_OK) return error;

  *events_handle = event_flags;
  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_eventflags_set(feabhOS_EVENTFLAGS * const events_handle,
                                     bitmask8_t                 flags_to_set)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(events_handle == NULL) return ERROR_INVALID_HANDLE;


  feabhOS_EVENTFLAGS event_flags = *events_handle;

  feabhOS_mutex_lock(&event_flags->mutex, WAIT_FOREVER);

  event_flags->flags |= flags_to_set;
  feabhOS_condition_notify_all(&event_flags->flag_set);

  feabhOS_mutex_unlock(&event_flags->mutex);
  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_eventflags_wait_all(feabhOS_EVENTFLAGS * const events_handle,
                                          bitmask8_t         * const flags_to_check,
                                          duration_mSec_t            timeout)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(events_handle == NULL) return ERROR_INVALID_HANDLE;
  if(flags_to_check == 0)   return ERROR_PARAM1;
  if(*flags_to_check == 0)  return ERROR_STUPID;

  feabhOS_EVENTFLAGS event_flags = *events_handle;

  feabhOS_mutex_lock(&event_flags->mutex, WAIT_FOREVER);


  while(((event_flags->flags & 0xFF) & (*flags_to_check & 0xFF)) != *flags_to_check)
  {
    feabhOS_error error;
    error = feabhOS_condition_wait(&event_flags->flag_set, &event_flags->mutex, timeout);
    if(error != ERROR_OK)
    {
      *flags_to_check = event_flags->flags;
      feabhOS_mutex_unlock(&event_flags->mutex);
      return ERROR_TIMED_OUT;
    }
  }

  *flags_to_check = event_flags->flags;
  feabhOS_mutex_unlock(&event_flags->mutex);
  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_eventflags_wait_any(feabhOS_EVENTFLAGS * const events_handle,
                                          bitmask8_t         * const flags_to_check,
                                          duration_mSec_t            timeout)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(events_handle == NULL) return ERROR_INVALID_HANDLE;
  if(flags_to_check == 0)   return ERROR_PARAM1;
  if(*flags_to_check == 0)  return ERROR_STUPID;

  feabhOS_EVENTFLAGS event_flags = *events_handle;

  feabhOS_mutex_lock(&event_flags->mutex, WAIT_FOREVER);

  // AND the current bit pattern with the bits to check.
  // If any match this will yield a non-zero result.
  //
  while(((event_flags->flags & 0xFF) & (*flags_to_check & 0xFF)) == 0)
  {
    feabhOS_error error;
    error = feabhOS_condition_wait(&event_flags->flag_set, &event_flags->mutex, timeout);
    if(error != ERROR_OK)
    {
      *flags_to_check = event_flags->flags;
      feabhOS_mutex_unlock(&event_flags->mutex);
      return ERROR_TIMED_OUT;
    }
  }

  *flags_to_check = event_flags->flags;
  feabhOS_mutex_unlock(&event_flags->mutex);
  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_eventflags_clear(feabhOS_EVENTFLAGS * const events_handle,
                                       bitmask8_t                 flags_to_clear)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(events_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_EVENTFLAGS event_flags = *events_handle;

  feabhOS_mutex_lock(&event_flags->mutex, WAIT_FOREVER);

  event_flags->flags &= ~flags_to_clear;

  feabhOS_mutex_unlock(&event_flags->mutex);
  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_eventflags_clear_all(feabhOS_EVENTFLAGS * const events_handle)
{
  return feabhOS_eventflags_clear(events_handle, 0xFF);
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_eventflags_destroy(feabhOS_EVENTFLAGS * const events_handle)
{
  // Parameter checking:
  //
  if(events_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_EVENTFLAGS event_flags = *events_handle;

  feabhOS_mutex_destroy(&event_flags->mutex);
  feabhOS_condition_destroy(&event_flags->flag_set);
  deallocate(event_flags);


  return ERROR_OK;
}

