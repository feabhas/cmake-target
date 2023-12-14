// feabhOS_condition.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef FEABHOS_CONDITION_H
#define FEABHOS_CONDITION_H

#include "feabhOS_errors.h"
#include "feabhOS_time.h"
#include "feabhOS_mutex.h"

#ifdef __cplusplus
extern "C" {
#endif


// -----------------------------------------------------------------------------------------------
// The condition handle.  You must create a handle for
// each condition object you want to manage.
//
typedef struct feabhOS_condition* feabhOS_CONDITION;


// -----------------------------------------------------------------------------------------------
// Create a condition object
// A condition acts a unidirectional, persistent, resetting
// signal object, with an interface to lock/unlock a mutex.
//
// Parameters:
// - condition_handle      A pointer to a feabhOS_CONDITION object
//
// Return values
// ERROR_OK                Success.  Condition handle will be non-NULL
// ERROR_OUT_OF_MEMORY     Could not allocate memory for the signal
//
feabhOS_error feabhOS_condition_create(feabhOS_CONDITION * const condition_handle);


// -----------------------------------------------------------------------------------------------
// Wait for condition
// Block the caller until another task signals.  The supplied mutex will be
// unlocked before the caller is suspended; and re-locked after the condition
// is signalled.
// If the condition wakes due to timeout expiration the mutex will be locked.
// The condition makes no guarantee against spurious wake-up.
// After wake-up there is no guarantee against pre-emption between the wake-up
// and re-acquisition of the mutex.
//
// Parameters:
// - condition_handle      A pointer to a feabhOS_CONDITION object
// - mutex_handle          A pointer to a feabhOS_MUTEX object
// - timeout               Specify the maximum time the caller will block
//                         May be set to
//                         NO_WAIT       - non-blocking wait
//                         WAIT_FOREVER  - block forever (only return on signal)
//
// Return values
// ERROR_OK                Success.  The condition was notified
// ERROR_TIMED_OUT         Condition timeout duration expired with no notification
// ERROR_INVALID_HANDLE    condition_handle == NULL
//
feabhOS_error feabhOS_condition_wait(feabhOS_CONDITION * const condition_handle,
                                     feabhOS_MUTEX     * const mutex_handle,
                                     duration_mSec_t           timeout);



// -----------------------------------------------------------------------------------------------
// Notify one task
// Wake one (the highest priority) task that is pending on
// the condition object.
//
// Parameters:
// - condition_handle      A pointer to a feabhOS_CONDITION object
//
// Return values
// ERROR_OK                Success.
// ERROR_INVALID_HANDLE    condition_handle == NULL
//
feabhOS_error feabhOS_condition_notify_one(feabhOS_CONDITION * const condition_handle);


// -----------------------------------------------------------------------------------------------
// Notify all tasks
// Wake all pending tasks that are pending on the condition.
//
// Parameters:
// - condition_handle      A pointer to a feabhOS_CONDITION object
//
// Return values
// ERROR_OK                Success.
// ERROR_INVALID_HANDLE    condition_handle == NULL
//
feabhOS_error feabhOS_condition_notify_all(feabhOS_CONDITION * const condition_handle);


// -----------------------------------------------------------------------------------------------
// Delete the condition
// Destroy the signal object and deallocate any memory for
// its management structure.
// Parameters:
// - condition_handle      A pointer to a feabhOS_CONDITION object
//
// Return values
// ERROR_OK                Success.
// ERROR_INVALID_HANDLE    condition_handle == NULL
//
feabhOS_error feabhOS_condition_destroy(feabhOS_CONDITION * const condition_handle);


#ifdef __cplusplus
}
#endif

#endif /* FEABHOS_CONDITION_H */
