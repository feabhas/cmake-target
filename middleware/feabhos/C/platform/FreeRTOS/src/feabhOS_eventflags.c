// feabhOS_eventflags.c
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#include <stdbool.h>
#include <assert.h>
#include "feabhOS_eventflags.h"
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
struct feabhOS_eventflags
{
  OS_EVENTFLAGS_TYPE handle;
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
  feabhOS_EVENTFLAGS eventflags = allocate();
  if(eventflags == NULL) return ERROR_OUT_OF_MEMORY;

  eventflags->handle = xEventGroupCreate();
  if(eventflags->handle == NULL) return ERROR_OUT_OF_MEMORY;

  *events_handle = eventflags;
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

  feabhOS_EVENTFLAGS eventflags = *events_handle;
  xEventGroupSetBits(eventflags->handle, (EventBits_t)flags_to_set);

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

  feabhOS_EVENTFLAGS eventflags = *events_handle;

  EventBits_t result = xEventGroupWaitBits(eventflags->handle,
                                           (EventBits_t)*flags_to_check,
                                           pdFALSE,
                                           pdTRUE,
                                           (OS_TIME_TYPE)timeout);

  // We don't know if all flags were set or if the call timed out.
  // The only way to know is to compare the result with the
  // provided bit pattern.
  //
  result &= 0x00FFu;
  if(*flags_to_check == result)
  {
    return ERROR_OK;
  }
  else
  {
    *flags_to_check = (bitmask8_t)result;
    return ERROR_TIMED_OUT;
  }
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

  feabhOS_EVENTFLAGS eventflags = *events_handle;

  EventBits_t result = xEventGroupWaitBits(eventflags->handle,
                                           (EventBits_t)*flags_to_check,
                                           pdFALSE,
                                           pdFALSE,
                                           (OS_TIME_TYPE)timeout);

  // We don't know if any of flags were set or if the call timed out.
  // The only way to know is to compare the result with the
  // provided bit pattern.
  //
  result &= 0x00FF;
  if((result & *flags_to_check) != 0)
  {
    *flags_to_check = (bitmask8_t)result;
    return ERROR_OK;
  }
  else
  {
    *flags_to_check = (bitmask8_t)result;
    return ERROR_TIMED_OUT;
  }
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

  feabhOS_EVENTFLAGS eventflags = *events_handle;
  xEventGroupClearBits(eventflags->handle, (EventBits_t)flags_to_clear);

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

  feabhOS_EVENTFLAGS eventflags = *events_handle;
  deallocate(eventflags);

  (*events_handle)->handle = NULL;

  return ERROR_OK;
}

