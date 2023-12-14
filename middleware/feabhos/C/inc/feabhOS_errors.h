// feabhOS_errors.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
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
