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

#ifndef FEABHOS_TASK_H
#define FEABHOS_TASK_H

#include "feabhOS_port_defs.h"
#include "feabhOS_errors.h"
#include "feabhOS_time.h"

#ifdef __cplusplus
extern "C" {
#endif


// -----------------------------------------------------------------------------------------------
// Task stack size.
// These sizes are not absolute values
// as they depend on the underlying OS.  See feabhOS_port_defs.h
// for values for this OS.
//
typedef enum
{
  STACK_TINY   = OS_STACK_TINY,
  STACK_SMALL  = OS_STACK_SMALL,
  STACK_NORMAL = OS_STACK_NORMAL,
  STACK_LARGE  = OS_STACK_LARGE,
  STACK_HUGE   = OS_STACK_HUGE
} feabhOS_stack_size_t;

// -----------------------------------------------------------------------------------------------
// Task priority.
// These values are not absolute values
// as they depend on the underlying OS.  See feabhOS_port_defs.h
// for values for this OS.
//
typedef enum
{
  PRIORITY_LOWEST  = OS_PRIORITY_LOWEST,
  PRIORITY_LOW     = OS_PRIORITY_LOW,
  PRIORITY_NORMAL  = OS_PRIORITY_NORMAL,
  PRIORITY_HIGH    = OS_PRIORITY_HIGH,
  PRIORITY_HIGHEST = OS_PRIORITY_HIGHEST
} feabhOS_priority_t;

// -----------------------------------------------------------------------------------------------
// The task handle.  You must create a handle for
// each task you want to manage.
//
typedef struct feabhOS_task* feabhOS_TASK;

// -----------------------------------------------------------------------------------------------
// Create a task
// This function creates a task in the READY state.
//
// Parameters:
// - task_handle           A pointer to a feabhOS_TASK object
// - function              A pointer to the task's function
// - param                 A user-specific pointer; passed to the task function
//                         on task start-up. Set to NULL if not required.
// - stack                 An enum value defining the task's stack
// - priority              An enum value defining the task's priority
//
// Return values
// ERROR_OK                Success.  Task handle will be non-NULL
// ERROR_OUT_OF_MEMORY     Could not allocate memory for the task
// ERROR_PARAM1            The function pointer was NULL
// ERROR_PARAM3            The stack was invalid
// ERROR_PARAM4            The priority was invalid.
//
feabhOS_error feabhOS_task_create(feabhOS_TASK * const  task_handle,
                                  void (*function)(void*),
                                  void *                param,
                                  feabhOS_stack_size_t  stack,
                                  feabhOS_priority_t    priority);


// -----------------------------------------------------------------------------------------------
// Set task priority
// Change the task's priority to the new value.
//
// Parameters:
// - task_handle            A pointer to a feabhOS_TASK object
// - priority               An enum value defining the task's priority
//
// Return values
// ERROR_OK                 Success
// ERROR_INVALID_HANDLE     The task's handle was NULL
// ERROR_PARAM1             The priority was invalid
// ERROR_STUPID             Attempting to modify terminated task
//
feabhOS_error feabhOS_task_setPriority(feabhOS_TASK * const task_handle, feabhOS_priority_t prio);


// -----------------------------------------------------------------------------------------------
// Wait for a task to finish
// The task calling join() will be suspended until the specified task
// has completed.
// If the task has completed, but is still in a joinable state a call
// to join() will succeed (and return immediately).
// Calling join() on an already-joined task will fail.
// Calling join() on a detached task will fail.
// If join fails the call will return immediately.
//
// Parameters:
// - task_handle            A pointer to a feabhOS_TASK object
//
// Return values
// ERROR_OK                 The specified task has completed.
// ERROR_INVALID_HANDLE     The task's handle was NULL
// ERROR_NOT_JOINABLE       The specified task has already been
//                          joined; or detached.
//
feabhOS_error feabhOS_task_join(feabhOS_TASK * const task_handle);


// -----------------------------------------------------------------------------------------------
// Detach a task
// Detaching a task puts it in a non-joinable state.  You cannot
// wait for a detached task (with join())
//
// Parameters:
// - task_handle            A pointer to a feabhOS_TASK object
//
// Return values
// ERROR_OK                 Success.
// ERROR_INVALID_HANDLE     The task's handle was NULL
// ERROR_STUPID             Attempting to detach a terminated task
//
feabhOS_error feabhOS_task_detach(feabhOS_TASK * const task_handle);


// -----------------------------------------------------------------------------------------------
// Destroy a task.
// After being destroyed the task can no longer be used.
//
// Parameters:
// - task_handle            A pointer to a feabhOS_TASK object
//
// Return values
// ERROR_OK                 Success.  Task handle will be NULL
// ERROR_INVALID_HANDLE     The task's handle was NULL
//
feabhOS_error feabhOS_task_destroy(feabhOS_TASK * const task_handle);


// -----------------------------------------------------------------------------------------------
// Suspend a task.
// The task is placed into the SUSPENDED state until it is
// explicitly resumed by a call to feabhOS_task_resume.
// A task may self-suspend by supplying its own task handle as
// a parameter. However, a task cannot self-resume - resumption
// must be done by another task.
//
// Parameters:
// - task_handle            A pointer to a feabhOS_TASK object
//
// Return values
// ERROR_OK                 Success
// ERROR_INVALID_HANDLE     The task's handle was NULL
// ERROR_STUPID             Attempting to suspend a terminated task
//
feabhOS_error feabhOS_task_suspend(feabhOS_TASK * const task_handle);


// -----------------------------------------------------------------------------------------------
// Resume a task.
// The task to be resumed must be in the SUSPENDED state.
// Calling resume on an already-running task will have no effect.
//
// Parameters:
// - task_handle            A pointer to a feabhOS_TASK object
//
// Return values
// ERROR_OK                 Success
// ERROR_INVALID_HANDLE     The task's handle was NULL
// ERROR_STUPID             Attempting to resume a terminated task
//
feabhOS_error feabhOS_task_resume(feabhOS_TASK * const task_handle);


// -----------------------------------------------------------------------------------------------
// Sleep
// Suspend a task for the specified duration. The calling task is the
// one that will be suspended; there is no need to supply a task handle.
//
// Parameters:
// - period                 Duration to suspend task for
//
// Return values
// None
//
void feabhOS_task_sleep(duration_mSec_t period);


// -----------------------------------------------------------------------------------------------
// Yield
// Yield the current task and force a re-schedule.  This function
// is semantically equivalent to sleep(0)
//
// Parameters:
// None
//
// Return values
// None
//
void feabhOS_task_yield(void);


#ifdef __cplusplus
}
#endif

#endif /* FEABHOS_TASK_H */
