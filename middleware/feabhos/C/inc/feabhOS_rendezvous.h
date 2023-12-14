// feabhOS_rendezvous.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
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

