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

#include "Thread.h"
#include "Scheduler.h"

namespace FeabhOS
{
  Thread::Thread() :
    runnable(NULL),
    handle(0),
    priority(DefaultPriority),
    stack(DefaultStack),
    done(true),
    created(false)
  {
  }


  Thread::Thread(Thread::Priority prio) :
    runnable(NULL),
    handle(0),
    priority(prio),
    stack(DefaultStack),
    done(true),
    created(false)
  {
  }


  Thread::Thread(Thread::Stack stacksize) :
    runnable(NULL),
    handle(0),
    priority(DefaultPriority),
    stack(stacksize),
    done(true),
    created(false)
  {
  }


  Thread::Thread(Thread::Priority prio, Thread::Stack stacksize) :
    runnable(NULL),
    handle(0),
    priority(prio),
    stack(stacksize),
    done(true),
    created(false)
  {
  }


  Thread::~Thread()
  {
    if(created)
    {
      // If the thread is joinable, we should wait for
      // it to finish before terminating.  This is the
      // safest option.
      // If the Thread has been detached (or already joined)
      // we will terminate the task, since we shouldn't have
      // OS threads in the system with no way to manage them.
      //
      feabhOS_task_join(&handle);
      feabhOS_task_destroy(&handle);
    }
  }


  void Thread::attach(I_Runnable& obj)
  {
    feabhOS_error error;

    // Check to see if start has already been called on this thread
    // object. If this is the case the original OS thread would be leaked!
    // There is no simple way to recover from this, so throw an exception.
    //
    if(created) throw ThreadAlreadyCreated();

    runnable = &obj;

    error = feabhOS_task_create(&handle,
                                &Thread::scheduledFunction,
                                reinterpret_cast<Thread*>(this),
                                static_cast<feabhOS_stack_size_t>(stack),
                                static_cast<feabhOS_priority_t>(priority));

    created = (error == ERROR_OK);
  }
 

  void Thread::scheduledFunction(void* arg)
  {
    Thread* thisThread = (reinterpret_cast<Thread*>(arg));
    thisThread->schedulePolicy();
  }


  void Thread::schedulePolicy()
  {
    do
    {
      done = runnable->run();
    } while(!done);
  }


  void Thread::join()
  {
    if(created)
    {
      feabhOS_error err = feabhOS_task_join(&handle);
      if(err == ERROR_NOT_JOINABLE) throw ThreadNotJoinable();
    }
  }


  void Thread::detach()
  {
    if(created)
    {
      feabhOS_task_detach(&handle);
    }
  }


  void Thread::suspend()
  {
    feabhOS_task_suspend(&handle);
  }


  void Thread::resume()
  {
    feabhOS_task_resume(&handle);
  }


  void Thread::sleep(duration_mSec_t period)
  {
    feabhOS_task_sleep(period);
  }


  void Thread::yield()
  {
    feabhOS_task_yield();
  }

} // namespace FeabhOS
