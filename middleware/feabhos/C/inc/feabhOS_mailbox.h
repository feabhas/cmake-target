// feabhOS_mailbox.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef FEABHOS_MAILBOX_H
#define FEABHOS_MAILBOX_H

#include <stdbool.h>
#include "feabhOS_errors.h"
#include "feabhOS_time.h"
#include "feabhOS_stdint.h"

#ifdef __cplusplus
extern "C" {
#endif


// -----------------------------------------------------------------------------------------------
// The signal handle.  You must create a handle for
// each signal you want to manage.
//
typedef struct feabhOS_mailbox* feabhOS_MAILBOX;


// -----------------------------------------------------------------------------------------------
// Create a mailbox
// Mailboxes are single-item blocking data storage.  A mailbox
// has overwrite semantics - that is, when data is put in the
// mailbox any existing data is over-written.
//
// Parameters:
// - mailbox_handle        A pointer to a feabhOS_MAILBOX object
// - elem_size             The size of each queue element (in bytes)
//
// Return values
// ERROR_OK                Success.  Mailbox handle will be non-NULL
// ERROR_OUT_OF_MEMORY     Could not allocate memory for the queue
// ERROR_PARAM1            elem_size  == 0
//
feabhOS_error feabhOS_mailbox_create(feabhOS_MAILBOX * const mailbox_handle,
                                     size_bytes_t            elem_size);


// -----------------------------------------------------------------------------------------------
// Insert into a mailbox.
// Data is copied into the mailbox.  If there is data in
// the mailbox it will be over-written.
//
// Parameters:
// - mailbox_handle        A pointer to a feabhOS_MAILBOX object
// - in                    A pointer to the data
//
// Return values
// ERROR_OK                Success.  Data is inserted into mailbox.
// ERROR_INVALID_HANDLE    mailbox_handle == NULL
// ERROR_PARAM1            in == NULL
//
feabhOS_error feabhOS_mailbox_post(feabhOS_MAILBOX * const mailbox_handle,
                                   void            * const in);


// -----------------------------------------------------------------------------------------------
// Retrieve from a mailbox.
// Data is copied into the in-out parameter.  If the mailbox is empty
// the caller will be suspended until there is data.
//
// Parameters:
// - mailbox_handle        A pointer to a feabhOS_MAILBOX object
// - in_out                A pointer to an object that will receive the data.
// - timeout               Specify the maximum time the caller will block
//                         May be set to
//                         NO_WAIT       - non-blocking wait
//                         WAIT_FOREVER  - block forever (only return on insertion)
//
// Return values
// ERROR_OK                Success.  Data will be pointed to by in_out.
// ERROR_QUEUE_EMPTY       Timeout duration expired.  No data retrieved.
// ERROR_INVALID_HANDLE    queue_handle == NULL
// ERROR_PARAM1            in_out == NULL
//
feabhOS_error feabhOS_mailbox_get(feabhOS_MAILBOX * const mailbox_handle,
                                  void            * const in_out,
                                  duration_mSec_t         timeout);


// -----------------------------------------------------------------------------------------------
// Check if the mailbox has data.
//
// Parameters:
// - mailbox_handle        A pointer to a feabhOS_MAILBOX object
//
// Return value
// true                    The mailbox is empty.
//                         An invalid mailbox will always be empty.
// false                   The mailbox has data
//
bool feabhOS_mailbox_is_empty(feabhOS_MAILBOX * const mailbox_handle);


// -----------------------------------------------------------------------------------------------
// Delete the mailbox
// Destroy the mailbox object and deallocate any memory for
// its management structure.
//
// Parameters:
// - mailbox_handle        A pointer to a feabhOS_QUEUE object
//
// Return values
// ERROR_OK                Success.
// ERROR_INVALID_HANDLE    mailbox_handle == NULL
//
feabhOS_error feabhOS_mailbox_destroy(feabhOS_MAILBOX * const mailbox_handle);


#ifdef __cplusplus
}
#endif


#endif // FEABHOS_MAILBOX_H_
