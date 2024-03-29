// feabhOS_rwlock.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef FEABHOS_RWLOCK_H
#define FEABHOS_RWLOCK_H

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
typedef struct feabhOS_rwlock* feabhOS_RWLOCK;


// -----------------------------------------------------------------------------------------------
// Create a rw-locks.
// A Multiple-Reader-Multiple-Writer lock (rw-lock) provides
// unlocked access to readers, but serialises access for writers.
//
// Parameters:
// - rwlock_handle         A pointer to a feabhOS_RWLOCK object
//
// Return values
// ERROR_OK                Success.  Queue handle will be non-NULL
// ERROR_OUT_OF_MEMORY     Could not allocate memory for the queue
//
feabhOS_error feabhOS_rwlock_create(feabhOS_RWLOCK * const rwlock_handle);

// -----------------------------------------------------------------------------------------------
// Acquire reader lock
// This lock must be acquired before attempting to read
// the (protected) resource.
// If another task is currently writing the caller will
// be suspended until ALL writers have completed.
//
// Parameters:
// - rwlock_handle         A pointer to a feabhOS_QUEUE object
//
// Return values
// ERROR_OK                Success. reader lock is acquired.
// ERROR_INVALID_HANDLE    rwlock_handle == NULL
//
feabhOS_error feabhOS_rwlock_read_acquire(feabhOS_RWLOCK * const rwlock_handle);


// -----------------------------------------------------------------------------------------------
// Release reader lock
// The lock must be released after a read.
// While there are active readers any tasks waiting to
// write will be blocked.
//
// Parameters:
// - rwlock_handle         A pointer to a feabhOS_QUEUE object
//
// Return values
// ERROR_OK                Success. reader lock is released.
// ERROR_INVALID_HANDLE    rwlock_handle == NULL
//
feabhOS_error feabhOS_rwlock_read_release(feabhOS_RWLOCK * const rwlock_handle);


// -----------------------------------------------------------------------------------------------
// Acquire writer lock
// The lock must be acquired before attempting to
// write to the (protected) resource.
// If any tasks are reading the caller will be blocked.
// If any tasks are currently writing the caller will be blocked.
//
// Parameters:
// - rwlock_handle         A pointer to a feabhOS_QUEUE object
//
// Return values
// ERROR_OK                Success. reader lock is released.
// ERROR_INVALID_HANDLE    rwlock_handle == NULL
//
feabhOS_error feabhOS_rwlock_write_acquire(feabhOS_RWLOCK * const rwlock_handle);


// -----------------------------------------------------------------------------------------------
// Release writer lock
// The lock must be released after a write.
// Any waiting writers will be woken.
// If there are no waiting writers any waiting
// readers will be unblocked.
//
// Parameters:
// - rwlock_handle         A pointer to a feabhOS_QUEUE object
//
// Return values
// ERROR_OK                Success. reader lock is released.
// ERROR_INVALID_HANDLE    rwlock_handle == NULL
//
feabhOS_error feabhOS_rwlock_write_release(feabhOS_RWLOCK * const rwlock_handle);


// -----------------------------------------------------------------------------------------------
// Delete the rw-lock
// Destroy the rw-lock object and deallocate any memory for
// its management structure.
//
// Parameters:
// - rwlock_handle         A pointer to a feabhOS_RWLOCK object
//
// Return values
// ERROR_OK                Success.
// ERROR_INVALID_HANDLE    rwlock_handle == NULL
//
feabhOS_error feabhOS_rwlock_destroy(feabhOS_RWLOCK * const rwlock_handle);


#ifdef __cplusplus
}
#endif

#endif // FEABHOS_RWLOCK_H_
