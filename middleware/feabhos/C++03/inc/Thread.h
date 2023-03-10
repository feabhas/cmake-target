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

#ifndef CPP03_FEABHOS_THREAD_H
#define CPP03_FEABHOS_THREAD_H

#include <stdexcept>

#include "feabhOS_task.h"
#include "feabhOS_time.h"

// -------------------------------------------------------------------------------------
// The FeabhOS::Signal class provides an implementation of the
// Thread-Runs-Polymorphic-Object pattern.
// -------------------------------------------------------------------------------------

namespace FeabhOS
{
  class I_Runnable
  {
  public:
    virtual bool run() = 0;
    virtual ~I_Runnable() {}
  };

  // ------------------------------------------------------------------------------
  // Thread exceptions
  //
  class ThreadCreationFailed : public std::runtime_error
  {
  public:
    ThreadCreationFailed(const char* str) : std::runtime_error(str) {}
    ThreadCreationFailed() : std::runtime_error("Thread creation failed!") {}
  };


  class ThreadAlreadyCreated : public std::runtime_error
  {
  public:
    ThreadAlreadyCreated(const char* str) : std::runtime_error(str) {}
    ThreadAlreadyCreated() : std::runtime_error("Thread already running!") {}
  };


  class ThreadNotJoinable : public std::runtime_error
  {
  public:
    ThreadNotJoinable(const char* str) : std::runtime_error(str) {}
    ThreadNotJoinable() : std::runtime_error("Thread not joinable") {}
  };


  // ------------------------------------------------------------------------------
  // Thread manages the underlying OS thread-of-control
  //
  class Thread
  {
  public:
    // ---------------------------------------------------------------
    // Thread stack and priority definitions
    //
    enum Priority
    {
      LowestPriority  = PRIORITY_LOWEST,
      LowPriority     = PRIORITY_LOW,
      NormalPriority  = PRIORITY_NORMAL,
      HighPriority    = PRIORITY_HIGH,
      HighestPriority = PRIORITY_HIGHEST,
      DefaultPriority = NormalPriority
    };

    enum Stack
    {
      TinyStack    = STACK_TINY,
      SmallStack   = STACK_SMALL,
      NormalStack  = STACK_NORMAL,
      LargeStack   = STACK_LARGE,
      HugeStack    = STACK_HUGE,
      DefaultStack = NormalStack
    };

    // ---------------------------------------------------------------
    // Thread operations.
    //
    Thread();
    Thread(Priority prio);
    Thread(Stack stacksize);
    Thread(Priority prio, Stack stacksize);
    virtual ~Thread();

    void attach(I_Runnable& obj);
    void join();
    void detach();

    void suspend();
    void resume();

    static void sleep(duration_mSec_t period);
    static void yield();

  private:
    // ---------------------------------------------------------------
    // This function is in the form required by
    // FeabhOS for thread creation.
    //
    static  void scheduledFunction(void* arg);
    virtual void schedulePolicy();

    // ---------------------------------------------------------------
    // Disable copying
    //
    Thread(const Thread&);
    Thread& operator= (const Thread&);

    // ---------------------------------------------------------------
    // The object to be run
    //
    I_Runnable* runnable;

    // ---------------------------------------------------------------
    // Thread management data:
    //
    feabhOS_TASK handle;
    Priority     priority;
    Stack        stack;
    bool         done;
    bool         created;
    // ---------------------------------------------------------------
  };

} // namespace FeabhOS

#endif // CPP03_FEABHOS_THREAD_H
