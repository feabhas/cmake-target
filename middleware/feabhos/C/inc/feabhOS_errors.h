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

#ifndef FEABHOS_ERRORS_H
#define FEABHOS_ERRORS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  ERROR_OK,                 // Success.
  
  // FATAL ERRORS
  //
  ERROR_OUT_OF_MEMORY,      // Memory cannot be allocated for the OS construct.
  ERROR_INVALID_HANDLE,     // The FeabhOS OS-construct is invalid (null)
  ERROR_PARAM1,             // First parameter (NOT the handle) is invalid.
  ERROR_PARAM2,             // Second parameter is invalid.
  ERROR_PARAM3,             // Third parameter is invalid.
  ERROR_PARAM4,             // Fourth parameter is invalid.
  ERROR_STUPID,             // An invalid call has been made.
  ERROR_UNKNOWN,            // A non-FeabhOS error has occurred.
  
  // NON-FATAL ERRORS
  //
  ERROR_TIMED_OUT,          // A blocking call has timed out.
  ERROR_QUEUE_EMPTY,        // No data on message queue.
  ERROR_QUEUE_FULL,         // Message queue is full.
  ERROR_NOT_JOINABLE,       // The task has already been joined (or has been detached)
  ERROR_MAX_COUNT,          // A counting semaphore has been 'given' max times.
  ERROR_NOT_OWNER           // Attempt to unlock a mutex that hasn't been locked.
  
} feabhOS_error;


const char* feabhOS_error_as_string(feabhOS_error err);


#ifdef __cplusplus
}
#endif

#endif /* FEABHOS_ERRORS_H */
