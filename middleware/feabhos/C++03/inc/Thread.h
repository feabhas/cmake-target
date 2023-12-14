// Thread.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
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
