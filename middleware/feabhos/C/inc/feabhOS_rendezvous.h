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

#ifndef FEABHOS_RENDEZVOUS_H
#define FEABHOS_RENDEZVOUS_H

#include "feabhOS_errors.h"
#include "feabhOS_time.h"

#ifdef __cplusplus
extern "C" {
#endif


// -----------------------------------------------------------------------------------------------
// The rendezvous handle.  You must create a handle for
// each rendezvous you want to manage.
//
typedef struct feabhOS_rendezvous* feabhOS_RENDEZVOUS;


// -----------------------------------------------------------------------------------------------
// Create a rendezvous
// A rendezvous represents a bidirectional, persistent, resetting
// synchronisation mechanism.
//
// Parameters:
// - rendezvous_handle         A pointer to a feabhOS_SIGNAL object
//
// Return values
// ERROR_OK                Success.  Signal handle will be non-NULL
// ERROR_OUT_OF_MEMORY     Could not allocate memory for the rendezvous
//
feabhOS_error feabhOS_rendezvous_create(feabhOS_RENDEZVOUS * const rendezvous_handle);


// -----------------------------------------------------------------------------------------------
// Call the rendezvous
// Signal the accepter that the first task is ready.
// If the accepter is already waiting continue immediately
// If the accepter hasn't accepted yet, suspend the caller.
//
// Parameters:
// - rendezvous_handle     A pointer to a feabhOS_RENDEZVOUS object
// - timeout               Specify the maximum time the caller will block
//                         May be set to
//                         NO_WAIT       - non-blocking wait
//                         WAIT_FOREVER  - block forever (only return on rendezvous)
//
// Return values
// ERROR_OK                Success.  The accepter notified.
// ERROR_TIMED_OUT         Signal timeout duration expired with no notification
// ERROR_INVALID_HANDLE    rendezvous_handle == NULL
//
feabhOS_error feabhOS_rendezvous_call(feabhOS_RENDEZVOUS * const rendezvous_handle,
                                      duration_mSec_t            timeout);


// -----------------------------------------------------------------------------------------------
// Accept the rendezvous
// Signal the caller that the second task is ready.
// If the first task has called, return immediately.
// If the caller hasn't called yet, suspend the accepter.
//
// Parameters:
// - rendezvous_handle     A pointer to a feabhOS_RENDEZVOUS object
// - timeout               Specify the maximum time the accepter will block
//                         May be set to
//                         NO_WAIT       - non-blocking wait
//                         WAIT_FOREVER  - block forever (only return on rendezvous)
//
// Return values
// ERROR_OK                Success.  The accepter notified.
// ERROR_TIMED_OUT         Signal timeout duration expired with no notification
// ERROR_INVALID_HANDLE    rendezvous_handle == NULL
//
feabhOS_error feabhOS_rendezvous_accept(feabhOS_RENDEZVOUS * const rendezvous_handle,
                                        duration_mSec_t            timeout);


// -----------------------------------------------------------------------------------------------
// Delete the rendezvous
// Destroy the rendezvous object and deallocate any memory for
// its management structure.
//
// Parameters:
// - rendezvous_handle     A pointer to a feabhOS_RENDEZVOUS object
//
// Return values
// ERROR_OK                Success.
// ERROR_INVALID_HANDLE    rendezvous_handle == NULL
//
feabhOS_error feabhOS_rendezvous_destroy(feabhOS_RENDEZVOUS * const rendezvous_handle);


#ifdef __cplusplus
}
#endif

#endif // FEABHOS_RENDEZVOUS_H

