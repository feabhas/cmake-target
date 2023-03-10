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

#ifndef FEABHOS_SEMAPHORE_H
#define FEABHOS_SEMAPHORE_H

#include "feabhOS_errors.h"
#include "feabhOS_time.h"
#include "feabhOS_stdint.h"

#ifdef __cplusplus
extern "C" {
#endif
  

// -----------------------------------------------------------------------------------------------
// The semaphore handle.  You must create a handle for
// each semaphore you want to manage.
//
typedef struct feabhOS_semaphore*   feabhOS_SEMAPHORE;

// -----------------------------------------------------------------------------------------------
// Create a semaphore.
// FeabhOS semaphores are counting semaphores.
//
// Parameters:
// - semaphore_handle      A pointer to a feabhOS_SEMAPHORE object
// - max_count             The maximum number of times the semaphore
//                         can be given.
//                         max_count = 1 simulates a binary semaphore.
// - init_count            The initial value for the count
//
// Return values
// ERROR_OK                Success.  Semaphore handle will be non-NULL
// ERROR_OUT_OF_MEMORY     Could not allocate memory for the semaphore
// ERROR_PARAM1            max_count == 0
// ERROR_PARAM2            init_count > max_count
//
feabhOS_error feabhOS_semaphore_create(feabhOS_SEMAPHORE * const semaphore_handle,
                                       num_elements_t            max_count,
                                       num_elements_t            init_count);


// -----------------------------------------------------------------------------------------------
// Take the semaphore.
// Taking a semaphore decrements the count value.  If the count is zero
// the caller will be blocked for the timeout period.
//
// Parameters:
// - semaphore_handle      A pointer to a feabhOS_SEMAPHORE object
// - timeout               Specify the maximum time the caller will block
//                         May be set to
//                         NO_WAIT       - non-blocking wait
//                         WAIT_FOREVER  - block forever (only return on signal)
//
// Return values
// ERROR_OK                Success.  The semaphore was taken
// ERROR_TIMED_OUT         Timeout duration expired with no take
// ERROR_INVALID_HANDLE    semaphore_handle == NULL
//
feabhOS_error feabhOS_semaphore_take(feabhOS_SEMAPHORE * const semaphore_handle, duration_mSec_t timeout);


// -----------------------------------------------------------------------------------------------
// Give the semaphore
// Giving the semaphore increments the count value.  If the count
// reaches the maximum the give operation will fail.
// Note: use feabhOS_semaphore_give_ISR() from within an ISR
//
// Parameters:
// - semaphore_handle      A pointer to a feabhOS_SIGNAL object
//
// Return values
// ERROR_OK                Success.
// ERROR_MAX_COUNT         The semaphore max count has been reached.
// ERROR_INVALID_HANDLE    signal_handle == NULL
//
feabhOS_error feabhOS_semaphore_give(feabhOS_SEMAPHORE * const semaphore_handle);

// -----------------------------------------------------------------------------------------------
// Give the semaphore from within an ISR
// Giving the semaphore increments the count value.  If the count
// reaches the maximum the give operation will fail.
//
// Parameters:
// - semaphore_handle      A pointer to a feabhOS_SIGNAL object
//
// Return values
// ERROR_OK                Success.
// ERROR_MAX_COUNT         The semaphore max count has been reached.
// ERROR_INVALID_HANDLE    signal_handle == NULL
//
feabhOS_error feabhOS_semaphore_give_ISR(feabhOS_SEMAPHORE * const semaphore_handle);
// -----------------------------------------------------------------------------------------------
// Delete the semaphore.
// Destroy the semaphore object and deallocate any memory for
// its management structure.
//
// Parameters:
// - semaphore_handle      A pointer to a feabhOS_SEMAPHORE object
//
// Return values
// ERROR_OK                Success.
// ERROR_INVALID_HANDLE    semaphore_handle == NULL
//
feabhOS_error feabhOS_semaphore_destroy(feabhOS_SEMAPHORE * const semaphore_handle);


#ifdef __cplusplus
}
#endif

#endif /* FEABHOS_SEMAPHORE_H */

