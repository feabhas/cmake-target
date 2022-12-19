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
#include "FreeRTOS.h"
#include "task.h"

#include "feabhOS_task.h"
#include "feabhOS_signal.h"

// ----------------------------------------------------------------------------
// Making a blocking call before the scheduler is started
// could lock up the entire system.  Therefore, any attempt
// to make a blocking call should assert.
//
extern bool scheduler_started;

// ----------------------------------------------------------------------------
// Static function prototypes
//
static void scheduled_function(void *arg);
static feabhOS_TASK allocate(void);
static void deallocate(feabhOS_TASK task);
static void terminate_task(feabhOS_TASK * const task_handle);


//  Although FeabhOS uses the same function signature as FreeRTOS this
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
  feabhOS_SIGNAL    join;
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
  feabhOS_error error;

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

  // Exit if we can't allocate a signal for this task
  //
  error = feabhOS_signal_create(&task->join);
  if(error != ERROR_OK) return ERROR_OUT_OF_MEMORY;

  task->user_code.function  = function;
  task->user_code.parameter = param;
  task->is_joinable         = true;

  OS_error = xTaskCreate(scheduled_function,
                         "FeabhOS task",
                         (uint16_t)(stack / sizeof(StackType_t)),
                         task,
                         (portBASE_TYPE)priority,
                         &task->handle);

  // The OS will fail if it cannot allocate memory
  // for (its own) control structures.
  //
  if(OS_error != pdPASS) return ERROR_OUT_OF_MEMORY;
  
  *task_handle = task;
  return ERROR_OK;
}

// ----------------------------------------------------------------------------
//
void scheduled_function(void *arg)
{
  feabhOS_TASK task = (feabhOS_TASK)arg;
  task->user_code.function(task->user_code.parameter);

  // At this point the user code is complete.
  // If the task is joinable we should signal any
  // waiting tasks that we've finished.
  // If the task is not joinable (that is, detached)
  // we can simply self-terminate.
  //
  if(task->is_joinable)
  {
    feabhOS_signal_notify_all(&task->join);
  }

  terminate_task(&task);
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

  feabhOS_signal_wait(&task->join, WAIT_FOREVER);
  task->is_joinable = false;
  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_task_detach(feabhOS_TASK * const task_handle)
{
  // Parameter checking:
  //
  if(task_handle == NULL)            return ERROR_INVALID_HANDLE;
  if((*task_handle)->handle == NULL) return ERROR_STUPID;

  feabhOS_TASK task = *task_handle;

  // Detached tasks can't be joined
  //
  task->is_joinable = false;
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
  if((*task_handle)->handle == NULL)                        return ERROR_STUPID;
  if((prio < PRIORITY_LOWEST) || (prio > PRIORITY_HIGHEST)) return ERROR_PARAM1;

  feabhOS_TASK task = *task_handle;
  vTaskPrioritySet((TaskHandle_t)&task->handle, (portBASE_TYPE)prio);

  return ERROR_OK;
}

// ----------------------------------------------------------------------------
//
static void terminate_task(feabhOS_TASK * const task_handle)
{
  // Remove the task from FreeRTOS, but do not
  // deallocate the memory for the task management
  // structure, in case anyone needs to access it
  //
  feabhOS_TASK task = *task_handle;

  if(task->handle != NULL)
  {
    OS_TASK_TYPE temp = task->handle;
    task->handle = NULL;

    vTaskDelete(temp);
  }
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_task_destroy(feabhOS_TASK * const task_handle)
{
  // Parameter checking:
  //
  if(task_handle == NULL)            return ERROR_INVALID_HANDLE;
  if((*task_handle)->handle == NULL) return ERROR_STUPID;

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
  if(task_handle == NULL)            return ERROR_INVALID_HANDLE;
  if((*task_handle)->handle == NULL) return ERROR_STUPID;

  feabhOS_TASK task = *task_handle;
  vTaskSuspend(task->handle);

  return ERROR_OK;
}


// ----------------------------------------------------------------------------
//
feabhOS_error feabhOS_task_resume(feabhOS_TASK * const task_handle)
{
  // Parameter checking:
  //
  if(task_handle == NULL)            return ERROR_INVALID_HANDLE;
  if((*task_handle)->handle == NULL) return ERROR_STUPID;

  feabhOS_TASK task = *task_handle;
  vTaskResume(task->handle);

  return ERROR_OK;
}

// ----------------------------------------------------------------------------
//
void feabhOS_task_sleep(duration_mSec_t period)
{
  portTickType delay = (OS_TIME_TYPE)(period) / portTICK_RATE_MS;
  vTaskDelay(delay);
}


// ----------------------------------------------------------------------------
//
void feabhOS_task_yield(void)
{
  feabhOS_task_sleep(0);
}



