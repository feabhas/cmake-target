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

#include <assert.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <stddef.h>

#include "feabhOS_task.h"
#include "feabhOS_condition.h"
#include "feabhOS_mutex.h"
#include "feabhOS_time_utils.h"

// ----------------------------------------------------------------------------
// Making a blocking call before the scheduler is started
// could lock up the entire system.  Therefore, any attempt
// to make a blocking call should assert.
//
extern bool scheduler_started;

// ----------------------------------------------------------------------------
// Static function prototypes
//
static void* scheduled_function(void *arg);
static feabhOS_TASK allocate(void);
static void deallocate(feabhOS_TASK task);
static void terminate_task(feabhOS_TASK * const task_handle);


//  Although FeabhOS uses the same function signature as POSIX this
//  is not always guaranteed.  Sometimes a port requires a different
//  function signature for its threads.  To hide this we  wrap the client's
//  function and parameter into a structure and pass this structure to an
//  intermediary function with the correct signature for the underlying OS.
//
struct user_code
{
  void (*function)(void*);
  void *parameter;
};


struct feabhOS_task
{
  OS_TASK_TYPE      handle;
  bool              is_joinable;
  struct user_code  user_code;
};


// ----------------------------------------------------------------------------
//
//	MEMORY MANAGEMENT FOR TASK STRUCTURES
//  -------------------------------------
//
//	For a fixed number of tasks we use a fixed-block
//  dynamic allocator.
//  If MAX_TASKS == NO_LIMIT we use the underlying OS'
//  dynamic memory allocator (usually malloc)
//

#if MAX_TASKS==NO_LIMIT

#include "feabhOS_memory.h"

static feabhOS_TASK allocate(void)
{
  return feabhOS_memory_alloc(sizeof(struct feabhOS_task));
}

static void deallocate(feabhOS_TASK task)
{
  feabhOS_memory_free(task);
}

#else

#include "feabhOS_allocator.h"

static struct feabhOS_task tasks[MAX_TASKS];
static feabhOS_POOL task_pool = NULL;


static feabhOS_TASK allocate(void)
{
  // Craete the pool first time a task is allocated
  //
  if(task_pool == NULL)
  {
    feabhOS_pool_create(&task_pool,
                        tasks,
                        sizeof(tasks),
                        sizeof(struct feabhOS_task),
                        MAX_TASKS);
  }

  return feabhOS_block_allocate(&task_pool);
}


static void deallocate(feabhOS_TASK task)
{
  feabhOS_block_free(&task_pool, task);
}

#endif

// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_task_create(feabhOS_TASK * const  task_handle,
                                  void (*function)(void*),
                                  void *                param, 
                                  feabhOS_stack_size_t  stack,
                                  feabhOS_priority_t    priority)
{
  feabhOS_TASK  task;
  OS_ERROR_TYPE OS_error;
  // feabhOS_error error;

  // Parameter checks:
  //
  if(function == NULL)                                              return ERROR_PARAM1;
  if((stack < STACK_TINY) || (stack > STACK_HUGE))                  return ERROR_PARAM3;
  if((priority < PRIORITY_LOWEST) || (priority > PRIORITY_HIGHEST)) return ERROR_PARAM4;
	
  // Exit if we couldn't allocate memory for the
  // task management structure
  //
  task = allocate();
  *task_handle = task;
  if(task == NULL) return ERROR_OUT_OF_MEMORY;

  task->user_code.function  = function;
  task->user_code.parameter = param;
  task->is_joinable         = true;

  pthread_attr_t task_attributes;
  pthread_attr_init(&task_attributes);
  pthread_attr_setstacksize(&task_attributes, stack);

  OS_error = pthread_create(&task->handle, 
                            &task_attributes,
                            scheduled_function,
                            (void*)task);

  // The OS will fail if it cannot allocate memory
  // for (its own) control structures.
  //
  if(OS_error != 0) return ERROR_OUT_OF_MEMORY;
  
  //*task_handle = task;
  return ERROR_OK;
}

// ----------------------------------------------------------------------------
//
void* scheduled_function(void *arg)
{
  feabhOS_TASK task = (feabhOS_TASK)arg;
  task->user_code.function(task->user_code.parameter);

  terminate_task(&task);

  return NULL;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_task_join(feabhOS_TASK * const task_handle)
{
  // Parameter checking:
  //
  assert(scheduler_started == true);
  if(task_handle == NULL) return ERROR_INVALID_HANDLE;

  feabhOS_TASK task = *task_handle;


  // Block until the specified task has completed
  // If the task has been detached (or join() has
  // already been called) return an error.
  //
  if(!task->is_joinable) return ERROR_NOT_JOINABLE;

  pthread_join(task->handle, NULL);

  task->is_joinable = false;
  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_task_detach(feabhOS_TASK * const task_handle)
{
  // Parameter checking:
  //
  if(task_handle == NULL)                          return ERROR_INVALID_HANDLE;
  if((*task_handle)->handle == (OS_TASK_TYPE)NULL) return ERROR_STUPID;

  feabhOS_TASK task = *task_handle;

  // Detached tasks can't be joined
  //
  task->is_joinable = false;
  pthread_detach(task->handle);

  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_task_setPriority(feabhOS_TASK * const task_handle,
                                       feabhOS_priority_t   prio)
{
  // Parameter checking:
  //
  if(task_handle == NULL)                                   return ERROR_INVALID_HANDLE;
  if((*task_handle)->handle == (OS_TASK_TYPE)NULL)          return ERROR_STUPID;
  if((prio < PRIORITY_LOWEST) || (prio > PRIORITY_HIGHEST)) return ERROR_PARAM1;

  feabhOS_TASK task = *task_handle;

  pthread_setschedprio(task->handle, prio);
  return ERROR_OK;
}

// ----------------------------------------------------------------------------
//
static void terminate_task(feabhOS_TASK * const task_handle)
{
  // Exit the current thread and inform any
  // Thread(s) joined on this thread.
  //
  (void)task_handle;

  pthread_exit(NULL);
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_task_destroy(feabhOS_TASK * const task_handle)
{
  // Parameter checking:
  //
  if(task_handle == NULL)                          return ERROR_INVALID_HANDLE;
  if((*task_handle)->handle == (OS_TASK_TYPE)NULL) return ERROR_STUPID;

  feabhOS_TASK task = *task_handle;

  terminate_task(&task);
  deallocate(task);

  return ERROR_OK;
}

// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_task_suspend(feabhOS_TASK * const task_handle)
{
  // Parameter checking:
  //
  if(task_handle == NULL)                          return ERROR_INVALID_HANDLE;
  if((*task_handle)->handle == (OS_TASK_TYPE)NULL) return ERROR_STUPID;
  (void)task_handle;
 
  // POSIX doesn't support thread suspension/resumption...
  //
  return ERROR_STUPID;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_task_resume(feabhOS_TASK * const task_handle)
{
  // Parameter checking:
  //
  if(task_handle == NULL)                          return ERROR_INVALID_HANDLE;
  if((*task_handle)->handle == (OS_TASK_TYPE)NULL) return ERROR_STUPID;

  (void)task_handle;
  
  // POSIX doesn't support thread suspension/resumption...
  //
  return ERROR_STUPID;
}

// ----------------------------------------------------------------------------
//
void feabhOS_task_sleep(duration_mSec_t period)
{
  struct timespec abs_period = to_timespec(period);
  nanosleep(&abs_period, NULL);
}


// ----------------------------------------------------------------------------
//
void feabhOS_task_yield(void)
{
  feabhOS_task_sleep(0);
}



