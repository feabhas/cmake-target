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
#ifndef CPP14_FEABHOS_MESSAGEQUEUE_H
#define CPP14_FEABHOS_MESSAGEQUEUE_H

#include <cstddef>
#include <stdexcept>

#include "FIFO.h"
#include "Mutex.h"
#include "Condition.h"
#include "Duration.h"

// -------------------------------------------------------------------------------------
// The FeabhOS::MessageQueue class(es) provide thread-safe FIFO
// storage for inter-thread communication
//
// Message queues have configurable properties that affect how
// the queues can be used.  It is possible to modify both post
// and retrieve behaviour to be either blocking or non-blocking
// as follows:
//
//           Blocking           Non-blocking
//           -------------------------------------------
// Post      Suspend on full    Throw exception if full
// Retrieve  Suspend on empty   Throw exception if empty
//
// Tag types are used to select the particular implementation:
//
//           Blocking           Non-blocking
//           --------------     ------------------
// Post      block_on_full      except_on_full
// Retrieve  block_on_empty     except_on_empty
//
// The default is a message queue that blocks on full and empty.
//
// The blocking characteristics affect the API that is available
// to the client as follows:
//
// Tag type        block_on_full      except_on_full     returns
//                 -------------      ---------------    ----------------------
// post()          Suspend on full    throw queue_full   void
//
// try_post()      Non-blocking       Non-blocking       true  => post succeeded
//                                                       false => queue full
//
// try_post_for()  Suspend on full        - -            true  => post succeeded
//                 (until timeout)                       false => timed out
//
//
// Tag type        block_on_empty     except_on_empty    returns
//                 -------------      ---------------    ----------------------
// get()           Suspend on empty   throw queue_empty  void
//
// try_get()       Non-blocking       Non-blocking       true  => get succeeded
//                                                       false => queue empty
//
// try_get_for()   Suspend on empty        - -           true  => get succeeded
//                 (until timeout)                       false => timed out
//
// -------------------------------------------------------------------------------------

namespace FeabhOS {

  // Tag classes for selecting
  // MessageQueue characteristics
  //
  class block_on_full   { };
  class except_on_full  { };
  class block_on_empty  { };
  class except_on_empty { };


  // Queue-specific exceptions
  //
  class queue_full : public std::out_of_range {
  public:
    queue_full(const char* str) : std::out_of_range(str) {}
    queue_full() : std::out_of_range("Message queue full!") {}
  };


  class queue_empty : public std::out_of_range {
  public:
    queue_empty(const char* str) : std::out_of_range(str) {}
    queue_empty() : std::out_of_range("Message queue empty!") {}
  };


  // -------------------------------------------------------------------------------------
  // Default MessageQueue
  //
  template<typename Message_Ty,
           std::size_t      sz,
           typename ReadPolicy  = block_on_empty,
           typename WritePolicy = block_on_full>
  class MessageQueue : private Utility::FIFO<Message_Ty, sz> {
  public:
    MessageQueue() = default;

    // Posting supports copy and move
    //
    template <typename T> void post(T&& in_msg);
    template <typename T> bool try_post(T&& in_msg);
    template <typename T> bool try_post_for(T&& in_msg, const Time::Duration& timeout);

    void get(Message_Ty& inout_msg);
    bool try_get(Message_Ty& inout_msg);
    bool try_get_for(Message_Ty& inout_msg, const Time::Duration& timeout);

    bool          is_empty() const;
    std::size_t   size()     const;
    std::size_t   capacity() const;

    // Copy / move policy
    //
    MessageQueue(const MessageQueue&)            = delete;
    MessageQueue& operator=(const MessageQueue&) = delete;
    MessageQueue(MessageQueue&&)                 = delete;
    MessageQueue& operator=(MessageQueue&&)      = delete;

private:
    using FIFO_t =  Utility::FIFO<Message_Ty, sz>;

    mutable Mutex mutex     { };
    Condition     has_data  { };
    Condition     has_space { };
  };
  

  template<typename Message_Ty,
           std::size_t sz,
           typename ReadPolicy,
           typename WritePolicy>
  template <typename T>
  void
  MessageQueue<Message_Ty, sz, ReadPolicy, WritePolicy>::post(T&& in_msg)
  {
    try_post_for(std::forward<T>(in_msg), Time::wait_forever);
  }


  template<typename Message_Ty,
           std::size_t sz,
           typename ReadPolicy,
           typename WritePolicy>
  template <typename T>
  bool
  MessageQueue<Message_Ty, sz, ReadPolicy, WritePolicy>::try_post(T&& in_msg)
  {
    return try_post_for(std::forward<T>(in_msg), Time::no_wait);
  }


  template<typename Message_Ty,
           std::size_t sz,
           typename ReadPolicy,
           typename WritePolicy>
  template <typename T>
  bool
  MessageQueue<Message_Ty, sz, ReadPolicy, WritePolicy>::try_post_for(T&& in_msg, const Time::Duration& timeout)
  {
    CRITICAL_SECTION(mutex)
    {
      while (FIFO_t::size() == sz) {
        if (!has_space.wait_for(mutex, timeout)) return false;
      }
      FIFO_t::add(std::forward<T>(in_msg));
      has_data.notify_all();
    }
    return true;
  }

  
  template<typename Message_Ty,
           std::size_t sz,
           typename ReadPolicy,
           typename WritePolicy>
  void
  MessageQueue<Message_Ty, sz, ReadPolicy, WritePolicy>::get(Message_Ty& inout_msg)
  {
    try_get_for(inout_msg, Time::wait_forever);
  }


  template<typename Message_Ty,
           std::size_t sz,
           typename ReadPolicy,
           typename WritePolicy>
  bool
  MessageQueue<Message_Ty, sz, ReadPolicy, WritePolicy>::try_get(Message_Ty& inout_msg)
  {
    return try_get_for(inout_msg, Time::no_wait);
  }


  template<typename Message_Ty,
           std::size_t sz,
           typename ReadPolicy,
           typename WritePolicy>
  bool
  MessageQueue<Message_Ty, sz, ReadPolicy, WritePolicy>::try_get_for(Message_Ty& inout_msg, const Time::Duration& timeout)
  {
    CRITICAL_SECTION(mutex)
    {
      while (FIFO_t::is_empty()) {
        if (!has_data.wait_for(mutex, timeout)) return false;
      }
      FIFO_t::get(inout_msg);
      has_space.notify_all();
    }
    return true;
  }


  template<typename Message_Ty,
           std::size_t sz,
           typename ReadPolicy,
           typename WritePolicy>
  bool
  MessageQueue<Message_Ty, sz, ReadPolicy, WritePolicy>::is_empty() const
  {
    bool empty { };

    CRITICAL_SECTION(mutex) { empty = FIFO_t::is_empty(); }
    return empty;
  }


  template<typename Message_Ty,
           std::size_t sz,
           typename ReadPolicy,
           typename WritePolicy>
  std::size_t
  MessageQueue<Message_Ty, sz, ReadPolicy, WritePolicy>::size() const
  {
    std::size_t size { };

    CRITICAL_SECTION(mutex) { size = FIFO_t::size(); }
    return size;
  }


  template<typename Message_Ty,
           std::size_t sz,
           typename ReadPolicy,
           typename WritePolicy>
  std::size_t
  MessageQueue<Message_Ty, sz, ReadPolicy, WritePolicy>::capacity() const
  {
    return sz;
  }


  // -------------------------------------------------------------------------------------
  // MessageQueue
  // Post:     except_on_full
  // Retrieve: block_on_empty
  //
  template<typename Message_Ty, std::size_t sz>
  class MessageQueue<Message_Ty, sz, block_on_empty, except_on_full>: private Utility::FIFO<Message_Ty, sz> {
  public:
    MessageQueue() = default;

    template <typename T> void post(T&& in_msg);
    template <typename T> bool try_post(T&& in_msg);

    void get(Message_Ty& inout_msg);
    bool try_get(Message_Ty& inout_msg);
    bool try_get_for(Message_Ty& inout_msg, const Time::Duration& timeout);

    bool          is_empty() const;
    std::size_t   size()     const;
    std::size_t   capacity() const;

    MessageQueue(const MessageQueue&)            = delete;
    MessageQueue& operator=(const MessageQueue&) = delete;
    MessageQueue(MessageQueue&&)                 = delete;
    MessageQueue& operator=(MessageQueue&&)      = delete;

private:
    using FIFO_t =  Utility::FIFO<Message_Ty, sz>;

    mutable Mutex mutex    { };
    Condition     has_data { };
  };


  template<typename Message_Ty, std::size_t sz>
  template <typename T>
  void
  MessageQueue<Message_Ty, sz, block_on_empty, except_on_full>::post(T&& in_msg)
  {
    if (!try_post(std::forward<T>(in_msg))) throw queue_full { };
  }


  template<typename Message_Ty, std::size_t sz>
  template <typename T>
  bool
  MessageQueue<Message_Ty, sz, block_on_empty, except_on_full>::try_post(T&& in_msg)
  {
    bool posted { };

    CRITICAL_SECTION(mutex)
    {
      posted = (FIFO_t::add(std::forward<T>(in_msg)) == FIFO_t::OK);

      if (posted) has_data.notify_all();
    }
    return posted;
  }


  template<typename Message_Ty, std::size_t sz>
  void
  MessageQueue<Message_Ty, sz, block_on_empty, except_on_full>::get(Message_Ty& inout_msg)
  {
    try_get_for(inout_msg, Time::wait_forever);
  }


  template<typename Message_Ty, std::size_t sz>
  bool
  MessageQueue<Message_Ty, sz, block_on_empty, except_on_full>::try_get(Message_Ty& inout_msg)
  {
    return try_get_for(inout_msg, Time::no_wait);
  }


  template<typename Message_Ty, std::size_t sz>
  bool
  MessageQueue<Message_Ty, sz, block_on_empty, except_on_full>::try_get_for(Message_Ty& inout_msg,
                                                                            const Time::Duration& timeout)
  {
    CRITICAL_SECTION(mutex)
    {
      while (FIFO_t::is_empty()) {
        if (!has_data.wait_for(mutex, timeout)) return false;
      }
      FIFO_t::get(inout_msg);
    }
    return true;
  }


  template<typename Message_Ty, std::size_t sz>
  bool
  MessageQueue<Message_Ty, sz, block_on_empty, except_on_full>::is_empty() const
  {
    bool empty { };

    CRITICAL_SECTION(mutex) { empty = FIFO_t::is_empty(); }
    return empty;
  }


  template<typename Message_Ty, std::size_t sz>
  std::size_t
  MessageQueue<Message_Ty, sz, block_on_empty, except_on_full>::size() const
  {
    std::size_t size { };

    CRITICAL_SECTION(mutex) { size = FIFO_t::size(); }
    return size;
  }


  template<typename Message_Ty, std::size_t sz>
  std::size_t
  MessageQueue<Message_Ty, sz, block_on_empty, except_on_full>::capacity() const
  {
    return sz;
  }


  // -------------------------------------------------------------------------------------
  // MessageQueue
  // Post:     block
  // Retrieve: exception
  //
  template<typename Message_Ty, std::size_t sz>
  class MessageQueue<Message_Ty, sz, except_on_empty, block_on_full>: private Utility::FIFO<Message_Ty, sz> {
  public:
    MessageQueue() = default;

    template <typename T> void post(T&& in_msg);
    template <typename T> bool try_post(T&& in_msg);
    template <typename T> bool try_post_for(T&& in_msg, const Time::Duration& timeout);

    void get(Message_Ty& inout_msg);
    bool try_get(Message_Ty& inout_msg);

    bool          is_empty() const;
    std::size_t   size()     const;
    std::size_t   capacity() const;

    MessageQueue(const MessageQueue&)            = delete;
    MessageQueue& operator=(const MessageQueue&) = delete;
    MessageQueue(MessageQueue&&)                 = delete;
    MessageQueue& operator=(MessageQueue&&)      = delete;

private:
    using FIFO_t =  Utility::FIFO<Message_Ty, sz>;

    mutable Mutex mutex     { };
    Condition     has_space { };
  };


  template<typename Message_Ty, std::size_t sz>
  template <typename T>
  void
  MessageQueue<Message_Ty, sz, except_on_empty, block_on_full>::post(T&& in_msg)
  {
    try_post_for(std::forward<T>(in_msg), Time::wait_forever);
  }


  template<typename Message_Ty, std::size_t sz>
  template <typename T>
  bool
  MessageQueue<Message_Ty, sz, except_on_empty, block_on_full>::try_post(T&& in_msg)
  {
    return try_post_for(std::forward<T>(in_msg), Time::no_wait);
  }


  template<typename Message_Ty, std::size_t sz>
  template <typename T>
  bool
  MessageQueue<Message_Ty, sz, except_on_empty, block_on_full>::try_post_for(T&& in_msg, const Time::Duration& timeout)
  {
    CRITICAL_SECTION(mutex)
    {
      while (FIFO_t::size() == sz) {
        if (!has_space.wait_for(mutex, timeout)) return false;
      }
      FIFO_t::add(std::forward<T>(in_msg));
    }
    return true;
  }


  template<typename Message_Ty, std::size_t sz>
  void
  MessageQueue<Message_Ty, sz, except_on_empty, block_on_full>::get(Message_Ty& inout_msg)
  {
    if (!try_get(inout_msg)) throw queue_empty { };
  }


  template<typename Message_Ty, std::size_t sz>
  bool
  MessageQueue<Message_Ty, sz, except_on_empty, block_on_full>::try_get(Message_Ty& inout_msg)
  {
    bool retrieved { };

    CRITICAL_SECTION(mutex)
    {
      retrieved = (FIFO_t::get(inout_msg) == FIFO_t::OK);

      if (retrieved) has_space.notify_all();
    }
    return retrieved;
  }


  template<typename Message_Ty, std::size_t sz>
  bool
  MessageQueue<Message_Ty, sz, except_on_empty, block_on_full>::is_empty() const
  {
    bool empty { };

    CRITICAL_SECTION(mutex) { empty = FIFO_t::is_empty(); }
    return empty;
  }


  template<typename Message_Ty, std::size_t sz>
  std::size_t
  MessageQueue<Message_Ty, sz, except_on_empty, block_on_full>::size() const
  {
    std::size_t size { };

    CRITICAL_SECTION(mutex) { size = FIFO_t::size(); }
    return size;
  }


  template<typename Message_Ty, std::size_t sz>
  std::size_t
  MessageQueue<Message_Ty, sz, except_on_empty, block_on_full>::capacity() const
  {
    return sz;
  }


  // -------------------------------------------------------------------------------------
  // MessageQueue
  // Post:     exception
  // Retrieve: exception
  //
  template<typename Message_Ty, std::size_t sz>
  class MessageQueue<Message_Ty, sz, except_on_empty, except_on_full>: private Utility::FIFO<Message_Ty, sz> {
  public:
    MessageQueue() = default;

    template <typename T> void post(T&& in_msg);
    template <typename T> bool try_post(T&& in_msg);

    void get(Message_Ty& inout_msg);
    bool try_get(Message_Ty& inout_msg);

    bool          is_empty() const;
    std::size_t   size()     const;
    std::size_t   capacity() const;

    MessageQueue(const MessageQueue&)            = delete;
    MessageQueue& operator=(const MessageQueue&) = delete;
    MessageQueue(MessageQueue&&)                 = delete;
    MessageQueue& operator=(MessageQueue&&)      = delete;

private:
    using FIFO_t =  Utility::FIFO<Message_Ty, sz>;

    mutable Mutex mutex { };
  };


  template<typename Message_Ty, std::size_t sz>
  template <typename T>
  void
  MessageQueue<Message_Ty, sz, except_on_empty, except_on_full>::post(T&& in_msg)
  {
    if (!try_post(std::forward<T>(in_msg))) throw queue_full { };
  }


  template<typename Message_Ty, std::size_t sz>
  template <typename T>
  bool
  MessageQueue<Message_Ty, sz, except_on_empty, except_on_full>::try_post(T&& in_msg)
  {
    bool posted { };

    CRITICAL_SECTION(mutex)
    {
      posted = (FIFO_t::add(std::forward<T>(in_msg)) == FIFO_t::OK);
    }
    return posted;
  }


  template<typename Message_Ty, std::size_t sz>
  void
  MessageQueue<Message_Ty, sz, except_on_empty, except_on_full>::get(Message_Ty& inout_msg)
  {
    if (!try_get(inout_msg)) throw queue_empty { };
  }


  template<typename Message_Ty, std::size_t sz>
  bool
  MessageQueue<Message_Ty, sz, except_on_empty, except_on_full>::try_get(Message_Ty& inout_msg)
  {
    bool retrieved { };

    CRITICAL_SECTION(mutex)
    {
      retrieved = (FIFO_t::get(inout_msg) == FIFO_t::OK);
    }
    return retrieved;
  }


  template<typename Message_Ty, std::size_t sz>
  bool
  MessageQueue<Message_Ty, sz, except_on_empty, except_on_full>::is_empty() const
  {
    bool empty { };

    CRITICAL_SECTION(mutex) { empty = FIFO_t::is_empty(); }
    return empty;
  }


  template<typename Message_Ty, std::size_t sz>
  std::size_t
  MessageQueue<Message_Ty, sz, except_on_empty, except_on_full>::size() const
  {
    std::size_t size { };

    CRITICAL_SECTION(mutex) { size = FIFO_t::size();}
    return size;
  }


  template<typename Message_Ty, std::size_t sz>
  std::size_t
  MessageQueue<Message_Ty, sz, except_on_empty, except_on_full>::capacity() const
  {
    return sz;
  }


} // namespace FeabhOS

#endif // CPP14_FEABHOS_MESSAGEQUEUE_H
