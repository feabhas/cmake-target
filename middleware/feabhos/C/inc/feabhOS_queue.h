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

#ifndef FEABHOS_QUEUE_H
#define FEABHOS_QUEUE_H

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
typedef struct feabhOS_queue* feabhOS_QUEUE;


// -----------------------------------------------------------------------------------------------
// Create a queue
// Queues are marshalling - that is, data is copied into the
// queue on insertion.
// Queues are fixed size - both the number of elements and the
// size of each element.
//
// Parameters:
// - queue_handle          A pointer to a feabhOS_QUEUE object
// - elem_size             The size of each queue element (in bytes)
// - queue_size            The number of elements in the queue
//
// Return values
// ERROR_OK                Success.  Queue handle will be non-NULL
// ERROR_OUT_OF_MEMORY     Could not allocate memory for the queue
// ERROR_PARAM1            elem_size  == 0
// ERROR_PARAM2            queue_size == 0
//
feabhOS_error feabhOS_queue_create(feabhOS_QUEUE * const queue_handle,
                                   size_bytes_t          elem_size,
                                   num_elements_t        queue_size);


// -----------------------------------------------------------------------------------------------
// Insert into a queue.
// Data is copied into the queue.  If the queue is full
// the caller will be suspended until there is space.
//
// Parameters:
// - queue_handle          A pointer to a feabhOS_QUEUE object
// - in                    A pointer to the data
// - timeout               Specify the maximum time the caller will block
//                         May be set to
//                         NO_WAIT       - non-blocking wait
//                         WAIT_FOREVER  - block forever (only return on insertion)
//
// Return values
// ERROR_OK                Success.  Data is inserted into queue.
// ERROR_QUEUE_FULL        Timeout duration expired.  Data is not inserted.
// ERROR_INVALID_HANDLE    queue_handle == NULL
// ERROR_PARAM1            in == NULL
//
feabhOS_error feabhOS_queue_post(feabhOS_QUEUE * const queue_handle,
                                 void          * const in,
                                 duration_mSec_t       timeout);


// -----------------------------------------------------------------------------------------------
// Retrieve from a queue.
// Data is copied into the in-out parameter.  If the queue is empty
// the caller will be suspended until there is data.
//
// Parameters:
// - queue_handle          A pointer to a feabhOS_QUEUE object
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
feabhOS_error feabhOS_queue_get(feabhOS_QUEUE * const queue_handle,
                                void          * const in_out,
                                duration_mSec_t       timeout);


// -----------------------------------------------------------------------------------------------
// Get the queue size
// The queue size is the number of elements currently
// in the queue, NOT the maximum number of elements
// it can hold (its capacity)
//
// Parameters:
// - queue_handle          A pointer to a feabhOS_QUEUE object
//
// Return value
// num_elements            The number of elements in the queue.
//                         num_elements = 0 if queue_handle == NULL
//
num_elements_t feabhOS_queue_size(feabhOS_QUEUE * const queue_handle);


// -----------------------------------------------------------------------------------------------
// Delete the queue
// Destroy the queue object and deallocate any memory for
// its management structure.
//
// Parameters:
// - queue_handle          A pointer to a feabhOS_QUEUE object
//
// Return values
// ERROR_OK                Success.
// ERROR_INVALID_HANDLE    queue_handle == NULL
//
feabhOS_error  feabhOS_queue_destroy(feabhOS_QUEUE * const queue_handle);


#ifdef __cplusplus
}
#endif

#endif /* FEABHOS_QUEUE_H */
