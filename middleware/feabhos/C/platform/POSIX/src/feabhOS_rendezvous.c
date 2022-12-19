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
#include "feabhOS_rendezvous.h"
#include "feabhOS_signal.h"
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
struct feabhOS_rendezvous
{
  feabhOS_SIGNAL caller;
  feabhOS_SIGNAL accepter;
};

// ----------------------------------------------------------------------------
//
//  MEMORY MANAGEMENT FOR RENDEZVOUS STRUCTURES
//  -------------------------------------------
//
//  For a fixed number of rendezvous we use a fixed-block
//  dynamic allocator.
//  If MAX_TASKS == NO_LIMIT we use the underlying OS'
//  dynamic memory allocator (usually malloc)
//

#if MAX_RENDEZVOUS==NO_LIMIT

#include "feabhOS_memory.h"

static feabhOS_RENDEZVOUS allocate(void)
{
  return feabhOS_memory_alloc(sizeof(struct feabhOS_rendezvous));
}


static void deallocate(feabhOS_RENDEZVOUS rendezvous)
{
  feabhOS_memory_free(rendezvous);
}

#else

#include "feabhOS_allocator.h"

static struct feabhOS_rendezvous rendezvous_mem[MAX_RENDEZVOUS];
static feabhOS_POOL rendezvous_pool = NULL;


static feabhOS_RENDEZVOUS allocate(void)
{
  // Craete the pool first time a task is allocated
  //
  if(rendezvous_pool == NULL)
  {
    feabhOS_pool_create(&rendezvous_pool,
                        rendezvous_mem,
                        sizeof(rendezvous_mem),
                        sizeof(struct feabhOS_rendezvous),
                        MAX_RENDEZVOUS);
  }

  return feabhOS_block_allocate(&rendezvous_pool);
}


static void deallocate(feabhOS_RENDEZVOUS rendezvous)
{
  feabhOS_block_free(&rendezvous_pool, rendezvous);
}

#endif

// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_rendezvous_create(feabhOS_RENDEZVOUS * const rendezvous_handle)
{
  feabhOS_RENDEZVOUS rendezvous = allocate();
  if(rendezvous == NULL) return ERROR_OUT_OF_MEMORY;

  feabhOS_error error;

  error = feabhOS_signal_create(&rendezvous->caller);
  if(error != ERROR_OK) return error;

  error = feabhOS_signal_create(&rendezvous->accepter);
  if(error != ERROR_OK) return error;

  *rendezvous_handle = rendezvous;
  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_rendezvous_call(feabhOS_RENDEZVOUS * const rendezvous_handle,
                                      duration_mSec_t            timeout)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(rendezvous_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_RENDEZVOUS rendezvous = *rendezvous_handle;
  feabhOS_error error;

  feabhOS_signal_notify_one(&rendezvous->caller);
  error = feabhOS_signal_wait(&rendezvous->accepter, timeout);

  return error;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_rendezvous_accept(feabhOS_RENDEZVOUS * const rendezvous_handle,
                                        duration_mSec_t            timeout)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(rendezvous_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_RENDEZVOUS rendezvous = *rendezvous_handle;
  feabhOS_error error;

  feabhOS_signal_notify_one(&rendezvous->accepter);
  error = feabhOS_signal_wait(&rendezvous->caller, timeout);

  return error;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_rendezvous_destroy(feabhOS_RENDEZVOUS * const rendezvous_handle)
{
  // Parameter checking:
  //
  if(rendezvous_handle == NULL) return ERROR_INVALID_HANDLE;

  // Ensure client's handle is invalid
  //
  feabhOS_RENDEZVOUS rendezvous = *rendezvous_handle;
  feabhOS_signal_destroy(&rendezvous->caller);
  feabhOS_signal_destroy(&rendezvous->accepter);
  deallocate(rendezvous);

  return ERROR_OK;
}
