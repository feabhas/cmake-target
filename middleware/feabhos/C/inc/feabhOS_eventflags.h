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

#ifndef FEABHOS_EVENTFLAG_H_
#define FEABHOS_EVENTFLAG_H_

#include "feabhOS_errors.h"
#include "feabhOS_time.h"
#include "feabhOS_stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------------------------
// The event flag handle.  You must create a handle for
// each set of event flags you want to manage.
//
typedef struct feabhOS_eventflags* feabhOS_EVENTFLAGS;


// -----------------------------------------------------------------------------------------------
// Create an event flag set.
// An event flag set represents a group of 8 unidirectional, persistent
// non-resetting synchronisation objects.  A task may pend on one or
// more of these flags to synchronise operations.
//
// Parameters:
// - events_handle         A pointer to a feabhOS_RWLOCK object
//
// Return values
// ERROR_OK                Success.  Event handle will be non-NULL
// ERROR_OUT_OF_MEMORY     Could not allocate memory for the event flag set
//
feabhOS_error feabhOS_eventflags_create(feabhOS_EVENTFLAGS * const events_handle);


// -----------------------------------------------------------------------------------------------
// Set event flags
// Each individual flag in an event set can be set individually.  Set
// flags remain set until they are manually cleared.
// Flags are OR-ed on subsequent calls.  You must use ...clear() to
// reset a flag.
// The effect of setting a flag depends on the waiting task(s).
//
// Parameters:
// - rwlock_handle         A pointer to a feabhOS_QUEUE object
//
// Return values
// ERROR_OK                Success. flags are set as specified.
// ERROR_INVALID_HANDLE    events_handle == NULL
//
feabhOS_error feabhOS_eventflags_set(feabhOS_EVENTFLAGS * const events_handle,
                                     bitmask8_t                 flags_to_set);


// -----------------------------------------------------------------------------------------------
// Wait for all bits
// This call performs a conjunctive blocking waiting on all the specified
// flags.
// Any waiting tasks are made READY when all specified flags are set; the
// order of flag setting is not important.
// Flags remain set after the call returns; and must be manually reset.
// If flags have not been reset, calling wait() again will immediately
// return.
//
// Parameters:
// - events_handle         A pointer to a feabhOS_EVENTFLAGS object
// - flags_to_check        Each set bit represents a flag that must be checked.
//                         After the call, this value holds the bits that were
//                         actually set. This may not be the input set if the
//                         timeout occurred.
// - timeout               Specify the maximum time the caller will block
//                         May be set to
//                         NO_WAIT       - non-blocking wait
//                         WAIT_FOREVER  - block forever (only return on signal)
//
// Return values
// ERROR_OK                Success.  All specified bits have been set.
// ERROR_TIMED_OUT         Event timeout duration expired with no notification.
// ERROR_INVALID_HANDLE    events_handle == NULL
// ERROR_PARAM1            flags_to_check (pointer) == NULL
// ERROR_STUPID            *flags_to_check == 0x00 (never wake up!)
//
feabhOS_error feabhOS_eventflags_wait_all(feabhOS_EVENTFLAGS * const events_handle,
                                          bitmask8_t         * const flags_to_check,
                                          duration_mSec_t            timeout);


// -----------------------------------------------------------------------------------------------
// Wait for any bit
// Performs a disjunctive wait on the specified flags.  The caller will
// be suspended until one of the specified flags is set.  The call returns
// the actual flag that was set.
//
// Parameters:
// - events_handle         A pointer to a feabhOS_EVENTFLAGS object
// - flags_to_check        Each set bit represents a flag that must be checked.
//                         After the call, this value holds the bit(s) that were
//                         actually set.
// - timeout               Specify the maximum time the caller will block
//                         May be set to
//                         NO_WAIT       - non-blocking wait
//                         WAIT_FOREVER  - block forever (only return on signal)
//
// Return values
// ERROR_OK                Success.  One (or more) bits have been set.
// ERROR_TIMED_OUT         Event timeout duration expired with no notification.
// ERROR_INVALID_HANDLE    events_handle == NULL
// ERROR_PARAM1            flags_to_check (pointer) == NULL
// ERROR_STUPID            *flags_to_check == 0x00 (never wake up!)
//
feabhOS_error feabhOS_eventflags_wait_any(feabhOS_EVENTFLAGS * const events_handle,
                                          bitmask8_t         * const flags_to_check,
                                          duration_mSec_t            timeout);


// -----------------------------------------------------------------------------------------------
// Clear flags
// Manually reset specified flags.  Clearing a non-set flag has
// no effect.
//
// Parameters:
// - events_handle         A pointer to a feabhOS_EVENTFLAGS object
//
// Return values
// ERROR_OK                Success.
// ERROR_INVALID_HANDLE    events_handle == NULL
//
feabhOS_error feabhOS_eventflags_clear(feabhOS_EVENTFLAGS * const events_handle,
                                       bitmask8_t                 flags_to_clear);


// -----------------------------------------------------------------------------------------------
// Clear all bits
// A convenience function to reset all currently-set flags
//
// Parameters:
// - events_handle         A pointer to a feabhOS_EVENTFLAGS object
//
// Return values
// ERROR_OK                Success.
// ERROR_INVALID_HANDLE    events_handle == NULL
//
feabhOS_error feabhOS_eventflags_clear_all(feabhOS_EVENTFLAGS * const events_handle);


// -----------------------------------------------------------------------------------------------
// Delete the event flags
// Destroy the event flag set object and deallocate any memory for
// its management structure.
//
// Parameters:
// - events_handle         A pointer to a feabhOS_EVENTFLAGS object
//
// Return values
// ERROR_OK                Success.
// ERROR_INVALID_HANDLE    events_handle == NULL
//
feabhOS_error feabhOS_eventflags_destroy(feabhOS_EVENTFLAGS * const events_handle);


#ifdef __cplusplus
}
#endif


#endif // FEABHOS_EVENTFLAG_H_
