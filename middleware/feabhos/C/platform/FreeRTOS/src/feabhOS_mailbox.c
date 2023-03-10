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
#include "feabhOS_mailbox.h"
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
struct feabhOS_mailbox
{
  OS_MAILBOX_TYPE handle;
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

#if MAX_MAILBOXES==NO_LIMIT

#include "feabhOS_memory.h"

static feabhOS_MAILBOX allocate(void)
{
  return feabhOS_memory_alloc(sizeof(struct feabhOS_mailbox));
}


static void deallocate(feabhOS_MAILBOX mailbox)
{
  feabhOS_memory_free(mailbox);
}

#else

#include "feabhOS_allocator.h"

static struct feabhOS_mailbox mailboxes[MAX_MAILBOXES];
static feabhOS_POOL mailbox_pool = NULL;


static feabhOS_MAILBOX allocate(void)
{
  // Create the pool first time a task is allocated
  //
  if(mailbox_pool == NULL)
  {
    feabhOS_pool_create(&mailbox_pool,
                        mailboxes,
                        sizeof(mailboxes),
                        sizeof(struct feabhOS_mailbox),
                        MAX_MAILBOXES);
  }

  return feabhOS_block_allocate(&mailbox_pool);
}


static void deallocate(feabhOS_MAILBOX mailbox)
{
  feabhOS_block_free(&mailbox_pool, mailbox);
}

#endif

// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_mailbox_create(feabhOS_MAILBOX * const mailbox_handle,
                                   size_bytes_t              elem_size)
{
  feabhOS_MAILBOX mailbox = allocate();
  if(mailbox == NULL) return ERROR_OUT_OF_MEMORY;

  mailbox->handle = xQueueCreate((OS_UNSIGNED_TYPE)1, (OS_UNSIGNED_TYPE)elem_size);
  if(mailbox->handle == 0) return ERROR_OUT_OF_MEMORY;

  *mailbox_handle = mailbox;
  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_mailbox_post(feabhOS_MAILBOX * const mailbox_handle,
                                   void            * const in)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(mailbox_handle == NULL) return ERROR_INVALID_HANDLE;
  if(in == NULL)             return ERROR_PARAM1;

  feabhOS_MAILBOX mailbox = *mailbox_handle;

  // Ignore the return from xQueueOverWrite, as it can
  // only be pdPASS
  //
  xQueueOverwrite(mailbox->handle, in);

  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_mailbox_get(feabhOS_MAILBOX * const mailbox_handle,
                                  void            * const in_out,
                                  duration_mSec_t         timeout)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(mailbox_handle == NULL) return ERROR_INVALID_HANDLE;
  if(in_out == NULL)         return ERROR_PARAM1;

  feabhOS_MAILBOX mailbox = *mailbox_handle;

  OS_ERROR_TYPE OSError = xQueueReceive(mailbox->handle, in_out, (OS_TIME_TYPE)timeout);

  if(OSError == pdPASS) return ERROR_OK;
  else                  return ERROR_QUEUE_EMPTY;
}


// ----------------------------------------------------------------------------
//
bool feabhOS_MAILBOX_is_empty(feabhOS_MAILBOX * const mailbox_handle)
{
  // Parameter checking:
  // An invalid mailbox will always be empty!
  //
  if(mailbox_handle == NULL) return true;

  feabhOS_MAILBOX mailbox = *mailbox_handle;
  return (uxQueueMessagesWaiting(mailbox->handle) == 0);
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_mailbox_destroy(feabhOS_MAILBOX * const mailbox_handle)
{
  // Parameter checking:
  //
  if(mailbox_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_MAILBOX mailbox = *mailbox_handle;
  vQueueDelete(mailbox->handle);

  deallocate(mailbox);

  // Ensure the client's handle is invalid
  //
  mailbox->handle = NULL;

  return ERROR_OK;
}

