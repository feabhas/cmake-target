// feabhOS_signal.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef FEABHOS_SIGNAL_H
#define FEABHOS_SIGNAL_H

#include <feabhOS_port_defs.h>
#include "feabhOS_errors.h"
#include "feabhOS_time.h"

#ifdef __cplusplus
extern "C" {
#endif


// -----------------------------------------------------------------------------------------------
// The signal handle.  You must create a handle for
// each signal you want to manage.
//
typedef struct feabhOS_signal* feabhOS_SIGNAL;

// -----------------------------------------------------------------------------------------------
// Create a signal
// This function creates a unidirectional, persistent, resetting
// signal object.
//
// Parameters:
// - signal_handle         A pointer to a feabhOS_SIGNAL object
//
// Return values
// ERROR_OK                Success.  Signal handle will be non-NULL
// ERROR_OUT_OF_MEMORY     Could not allocate memory for the signal
//
feabhOS_error feabhOS_signal_create(feabhOS_SIGNAL * const signal_handle);


// -----------------------------------------------------------------------------------------------
// Wait for a signal
// Block the caller until another task signals
// (with ..notify_one or ...notify_all)
//
// Parameters:
// - signal_handle         A pointer to a feabhOS_SIGNAL object
// - timeout               Specify the maximum time the caller will block
//                         May be set to
//                         NO_WAIT       - non-blocking wait
//                         WAIT_FOREVER  - block forever (only return on signal)
//
// Return values
// ERROR_OK                Success.  The signal was notified
// ERROR_TIMED_OUT         Signal timeout duration expired with no notification
// ERROR_INVALID_HANDLE    signal_handle == NULL
//
feabhOS_error feabhOS_signal_wait(feabhOS_SIGNAL * const signal_handle, duration_mSec_t timeout);


// -----------------------------------------------------------------------------------------------
// Notify one task
// Wake one (the highest priority) task that is pending on
// the signal object.
// use feabhOS_signal_notify_one_ISR() inside an ISR
//
// Parameters:
// - signal_handle         A pointer to a feabhOS_SIGNAL object
//
// Return values
// ERROR_OK                Success.
// ERROR_INVALID_HANDLE    signal_handle == NULL
//
feabhOS_error feabhOS_signal_notify_one(feabhOS_SIGNAL * const signal_handle);

// -----------------------------------------------------------------------------------------------
// Notify one task (within ISR)
// Wake one (the highest priority) task that is pending on
// the signal object.
//
// Parameters:
// - signal_handle         A pointer to a feabhOS_SIGNAL object
//
// Return values
// ERROR_OK                Success.
// ERROR_INVALID_HANDLE    signal_handle == NULL
//
feabhOS_error feabhOS_signal_notify_one_ISR(feabhOS_SIGNAL * const signal_handle);

// -----------------------------------------------------------------------------------------------
// Notify all tasks
// Wake all pending tasks that are pending on the signal.
//
// Parameters:
// - signal_handle         A pointer to a feabhOS_SIGNAL object
//
// Return values
// ERROR_OK                Success.
// ERROR_INVALID_HANDLE    signal_handle == NULL
//
feabhOS_error feabhOS_signal_notify_all(feabhOS_SIGNAL * const signal_handle);


// -----------------------------------------------------------------------------------------------
// Delete the signal
// Destroy the signal object and deallocate any memory for
// its management structure.
//
// Parameters:
// - signal_handle         A pointer to a feabhOS_SIGNAL object
//
// Return values
// ERROR_OK                Success.
// ERROR_INVALID_HANDLE    signal_handle == NULL
//
feabhOS_error feabhOS_signal_destroy(feabhOS_SIGNAL * const signal_handle);


#ifdef __cplusplus
}
#endif

#endif /* FEABHOS_SIGNAL_H */

