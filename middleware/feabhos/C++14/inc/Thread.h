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

#ifndef CPP14_FEABHOS_THREAD_H
#define CPP14_FEABHOS_THREAD_H

#include <stdexcept>
#include <memory>
#include <cassert>
#include "feabhOS_task.h"
#include "Callback.h"
#include "Duration.h"

// -------------------------------------------------------------------------------------
// The FeabhOS::Thread class provides an implementation of the
// Thread-Runs-Callable-Object pattern.  From the client's perspective
// this class can be used in much the same way as the std::thread
// library class; although FeabhOS::Thread gives much finer-grained
// control over thread priority, stack size and thread management
// -------------------------------------------------------------------------------------

namespace FeabhOS {

  // ------------------------------------------------------------------------------
  // Thread exceptions
  //
  class thread_creation_failed : public std::runtime_error {
  public:
    thread_creation_failed(const char* str) : std::runtime_error(str) {}
    thread_creation_failed() : std::runtime_error("Thread creation failed!") {}
  };


  class thread_already_created : public std::runtime_error {
  public:
    thread_already_created(const char* str) : std::runtime_error(str) {}
    thread_already_created() : std::runtime_error("Thread already running!") {}
  };


  class thread_not_joinable : public std::runtime_error {
  public:
    thread_not_joinable(const char* str) : std::runtime_error(str) {}
    thread_not_joinable() : std::runtime_error("Thread not joinable!") {}
  };

  // ------------------------------------------------------------------------------
  // Thread stack and priority definitions
  //
  enum class Priority {
    Lowest  = PRIORITY_LOWEST,
    Low     = PRIORITY_LOW,
    Normal  = PRIORITY_NORMAL,
    High    = PRIORITY_HIGH,
    Highest = PRIORITY_HIGHEST
  };

  enum Stack {
    Tiny    = STACK_TINY,
    Small   = STACK_SMALL,
    Normal  = STACK_NORMAL,
    Large   = STACK_LARGE,
    Huge    = STACK_HUGE
  };


  /// ------------------------------------------------------------------------------
  // Thread manages the underlying OS thread-of-control
  //
  class Thread {
  public:
    // -----------------------------------------------------------------------------
    // Thread construction.
    // There are three basic options:
    // - If you are happy with the default priority and stack size
    //   you can construct a Thread with the behaviour it will run.
    //   Attempting to call attach() after this will fail.
    //
    // - Construct a thread, adjusting stack/priority as required.
    //   Then call attach() to add the behaviour.
    //
    // - Constructing an empty Thread and attaching no behaviour
    //   will do nothing.
    //
    Thread()                               = default;
    Thread(Priority prio)                  : priority { prio } {}
    Thread(Stack stacksize)                : stack { stacksize } {};
    Thread(Priority prio, Stack stacksize) : priority { prio }, stack { stacksize } {}

    template <typename Fn_Ty, typename... Param_Ty>
    inline
    Thread(Fn_Ty fn, Param_Ty&&... arg);

    // -----------------------------------------------------------------------------
    // Thread destruction
    // Destroying a Thread object will cause it to delete the
    // underlying OS thread.  The behaviour of your application
    // is undefined.
    //
    inline ~Thread();

    // -----------------------------------------------------------------------------
    // Thread behaviour
    // Calling attach() associates this Thread object with a callable
    // object (function).  In this implementation you can only
    // attach behaviour once.
    //
    template <typename Fn_Ty, typename... Param_Ty>
    inline
    void attach(Fn_Ty fn, Param_Ty&&... arg);

    // -----------------------------------------------------------------------------
    // Thread management.
    // - Adjusting Thread priority can be done at any time.
    //
    // - Adjusting stack size can only be done while no behavoiur
    //   is attached to this thread.  Attempting to change stack
    //   size after this will return an 'invalid call' error.
    //
    // - Calling suspend() / resume() on a Thread with no
    //   behaviour attached has no effect.
    //
    // - A client can wait for a thread to finish by calling
    //   join().  A thread can only be joined once.  calling
    //   join() on a joined thread will throw an exception.
    //
    // - If a thread is detached it cannot be joined.
    //
    inline feabhOS_error set_stack(Stack stck);
    inline Priority      get_priority();
    inline void          set_priority(Priority prio);
    inline void          suspend();
    inline void          resume();
    inline void          join();
    inline void          detach();

    // -----------------------------------------------------------------------------
    // Thread blocking calls
    // sleep() and yield() always affect the calling Thread object
    //
    static inline void sleep(Time::Duration period);
    static inline void yield();

    // -----------------------------------------------------------------------------
    // Thread copy and move policy.
    // Threads support moving but not copying.
    //
    Thread(const Thread&)             = delete;
    Thread& operator= (const Thread&) = delete;
    Thread(Thread&&)                  = default;
    Thread& operator= (Thread&&)      = default;

  protected:
    // -----------------------------------------------------------------------------
    // Internal utility functions
    //
    inline bool created();

  private:
    feabhOS_TASK handle   { nullptr };
    Priority     priority { Priority::Normal };
    Stack        stack    { Stack::Normal };

    template <typename Callback_Ty>
    static void scheduled_function(void* arg);

    template <typename Fn_Ty, typename... Param_Ty>
    typename std::enable_if<!std::is_member_function_pointer<Fn_Ty>::value, void>::type
    create_OS_task(Fn_Ty fn, Param_Ty... arg);

    template <typename Fn_Ty, typename... Param_Ty>
    typename std::enable_if<std::is_member_function_pointer<Fn_Ty>::value, void>::type
    create_OS_task(Fn_Ty fn, Param_Ty... arg);
  };


  // -----------------------------------------------------------------------------
  // This function is supplied to the C API.  It is the function registered with
  // the underlying OS.  Note, even though it is a template function it has the
  // same call signature as a normal C function.
  // If this function exits it cannot be restarted.
  // The scheduled_function() is responsible for managing the lifetime of the
  // Callback object. Once this function exits the Callback will be deleted; and
  // any other tasks pending (joined) on this task will be signalled.
  //
  template <typename Callback_Ty>
  void Thread::scheduled_function(void* arg)
  {
    std::unique_ptr<Callback_Ty> callback_ptr { reinterpret_cast<Callback_Ty*>(arg) };
    try {
      (*callback_ptr)();
    }
    catch (...) {
      assert(false);
    }
  }

  // -----------------------------------------------------------------------------
  // The create_OS_task() function invokes the underlying OS (via a C API).  The
  // function creates an appropriate Callback object deduced from its parameters.
  // The Callback object must be dynamically created since its lifetime will
  // be longer than the create_OS_task() function. Notice that the lifetime of the
  // Callback object is not managed by this function; that responsibility is handed
  // over to the scheduled_function() (which manages it using a unique_ptr).
  //
  // The create_OS_task() function is overloaded (with the same API!) for normal
  // (free) functions and class-member functions.  std::enable_if is used to disable
  // invalid overloads (using SFINAE).
  //
  // If the thread can't be created for some reason (most likely not enough memory)
  // then tidy up  the Callback object and inform the client by throwing an exception.
  // Why an exception?
  // - We are using enable_if to provide different implementations for member and non-member
  //   functions.
  // - We can't use the template parameter list for enable_if as we have a default
  //   template parameter
  // - We can't use a default argument as we are using a variadic template argument list
  // - We must therefore use the return type for enable_if
  // - Thus there's no other way of returning an error!
  //

  // Using aliases to make the code a little more readable.
  //
  template <typename U>
  using enable_for_member_function = typename std::enable_if<std::is_member_function_pointer<U>::value, void>::type;

  template <typename U>
  using enable_for_functor  = typename std::enable_if<!std::is_member_function_pointer<U>::value, void>::type;


  template <typename Fn_Ty, typename... Param_Ty>
  enable_for_functor<Fn_Ty>
  Thread::create_OS_task(Fn_Ty fn, Param_Ty... arg)
  {
    if (created()) throw thread_already_created { };

    // Alias to simplify code
    //
    using Callback_Ty = FeabhOS::Utility::Callback<Fn_Ty, Param_Ty...>;

    Callback_Ty* callback_ptr = new Callback_Ty { fn, std::forward<Param_Ty>(arg)... };

    feabhOS_error error = feabhOS_task_create(&handle,
                                              reinterpret_cast<void(*)(void*)>(&Thread::scheduled_function<Callback_Ty>),
                                              reinterpret_cast<void*>(callback_ptr),
                                              static_cast<feabhOS_stack_size_t>(stack),
                                              static_cast<feabhOS_priority_t>(priority));

    if (error != ERROR_OK) {
      delete callback_ptr;
      throw thread_creation_failed { };
    }
  }


  template <typename Fn_Ty, typename... Param_Ty>
  enable_for_member_function<Fn_Ty>
  Thread::create_OS_task(Fn_Ty fn, Param_Ty... arg)
  {
    if (created()) throw thread_already_created { };

    // Note: Use std::mem_fn to convert the member function into a
    // functor.
    //
    using Callback_Ty = FeabhOS::Utility::Callback<decltype(std::mem_fn(fn)), Param_Ty...>;

    Callback_Ty* callback_ptr = new Callback_Ty { std::mem_fn(fn), std::forward<Param_Ty>(arg)... };

    feabhOS_error error = feabhOS_task_create(&handle,
                                              reinterpret_cast<void(*)(void*)>(&Thread::scheduled_function<Callback_Ty>),
                                              reinterpret_cast<void*>(callback_ptr),
                                              static_cast<feabhOS_stack_size_t>(stack),
                                              static_cast<feabhOS_priority_t>(priority));

    if (error != ERROR_OK) {
      delete callback_ptr;
      throw thread_creation_failed { };
    }
  }


  template <typename Fn_Ty, typename... Param_Ty>
  Thread::Thread(Fn_Ty fn, Param_Ty&&... arg)
  {
    create_OS_task(fn, std::forward<Param_Ty>(arg)...);
  }


  template <typename Fn_Ty, typename... Param_Ty>
  void Thread::attach(Fn_Ty fn, Param_Ty&&... arg)
  {
    create_OS_task(fn, std::forward<Param_Ty>(arg)...);
  }


  Thread::~Thread()
  {
    if (created()) feabhOS_task_destroy(&handle);
  }


  feabhOS_error Thread::set_stack(Stack stck)
  {
    if (created()) return ERROR_STUPID;

    stack = stck;
    return ERROR_OK;
  }


  Priority Thread::get_priority()
  {
    return priority;
  }


  void Thread::set_priority(Priority prio)
  {
    if (created()) {
      feabhOS_task_setPriority(&handle, static_cast<feabhOS_priority_t>(prio));
    }
    else {
      priority = prio;
    }
  }


  void Thread::suspend()
  {
    if (created()) feabhOS_task_suspend(&handle);
  }


  void Thread::resume()
  {
    if (created()) feabhOS_task_resume(&handle);
  }


  void Thread::join()
  {
    if (created()) {
      if (feabhOS_task_join(&handle) == ERROR_NOT_JOINABLE) {
        throw thread_not_joinable { };
      }
    }
  }


  void Thread::detach()
  {
    if (created()) feabhOS_task_detach(&handle);
  }


  void Thread::sleep(Time::Duration period)
  {
    feabhOS_task_sleep(period);
  }


  void Thread::yield()
  {
    feabhOS_task_yield();
  }


  bool Thread::created()
  {
    return (handle != nullptr);
  }

} // namespace FeabhOS


#endif //CPP14_FEABHOS_THREAD_H
