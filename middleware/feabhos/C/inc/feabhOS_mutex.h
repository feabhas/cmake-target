// feabhOS_mutex.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef FEABHOS_MUTEX_H
#define FEABHOS_MUTEX_H

#include "feabhOS_errors.h"
#include "feabhOS_time.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------------------------
// The mutex handle.  You must create a handle for
// each mutex you want to manage.
//
typedef struct feabhOS_mutex* feabhOS_MUTEX;


// -----------------------------------------------------------------------------------------------
// Create a mutex.
// FeabhOS mutexes support priority inheritance protocol.
// Mutexes are NOT recursive (beware!)
//
// Parameters:
// - mutex_handle          A pointer to a feabhOS_MUTEX object
//
// Return values
// ERROR_OK                Success.  Mutex handle will be non-NULL
// ERROR_OUT_OF_MEMORY     Could not allocate memory for the mutex
//
feabhOS_error feabhOS_mutex_create(feabhOS_MUTEX * const mutex_handle);

// -----------------------------------------------------------------------------------------------
// Lock the semaphore.
// If the mutex is currently locked by another task the caller will
// put in the SUSPENDED state until either the mutex is available
// or the timeout duration expires.
// Calling lock() on a mutex you currently hold will deadlock the task.
//
// Parameters:
// - mutex_handle          A pointer to a feabhOS_MUTEX object
// - timeout               Specify the maximum time the caller will block
//                         May be set to
//                         NO_WAIT       - non-blocking wait
//                         WAIT_FOREVER  - block forever (only return on signal)
//
// Return values
// ERROR_OK                Success.  The mutex was locked.
// ERROR_TIMED_OUT         Timeout duration expired with no lock acquisition.
// ERROR_INVALID_HANDLE    mutex_handle == NULL
//
feabhOS_error feabhOS_mutex_lock(feabhOS_MUTEX * const mutex_handle, duration_mSec_t timeout);


// -----------------------------------------------------------------------------------------------
// Unlock the mutex.
// Unlocking the mutex will release all waiting tasks.
// Attempting to unlock a mutex you have not locked will fail.
//
// Parameters:
// - semaphore_handle      A pointer to a feabhOS_SIGNAL object
//
// Return values
// ERROR_OK                Success; the lock is released.
// ERROR_NOT_OWNER         Attempting to unlock an lock you don't own
// ERROR_INVALID_HANDLE    mutex_handle == NULL
//
feabhOS_error feabhOS_mutex_unlock (feabhOS_MUTEX * const mutex_handle);


// -----------------------------------------------------------------------------------------------
// Delete the mutex.
// Destroy the mutex object and deallocate any memory for
// its management structure.
//
// Parameters:
// - mutex_handle      A pointer to a feabhOS_MUTEX object
//
// Return values
// ERROR_OK                Success.
// ERROR_INVALID_HANDLE    mutex_handle == NULL
//
feabhOS_error feabhOS_mutex_destroy(feabhOS_MUTEX * const mutex_handle);


#ifdef __cplusplus
}
#endif

#endif // FEABHOS_MUTEX_H



