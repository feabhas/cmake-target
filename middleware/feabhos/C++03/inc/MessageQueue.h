// MessageQueue.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef CPP03_FEABHOS_MESSAGEQUEUE_H
#define CPP03_FEABHOS_MESSAGEQUEUE_H

#include "Mutex.h"
#include "Condition.h"
#include "FIFO.h"
#include "feabhOS_time.h"

namespace FeabhOS
{
  template<typename MessageType = int, unsigned int sz = 8>
  class MessageQueue : private Utility::FIFO<MessageType, sz>
  {
  public:
    MessageQueue();
    feabhOS_error post(const MessageType& in_msg, duration_mSec_t timeout = WAIT_FOREVER);
    feabhOS_error get(MessageType& inout_msg, duration_mSec_t timeout = WAIT_FOREVER);
    bool isEmpty();

  private:
    MessageQueue(const MessageQueue&);
    MessageQueue& operator=(const MessageQueue&);

    typedef Utility::FIFO<MessageType, sz> FIFOBase;
    Mutex mutex;
    Condition notEmpty;
    Condition notFull;
  };


  template<typename MessageType, unsigned int sz>
  MessageQueue<MessageType, sz>::MessageQueue() :
    FIFOBase<MessageType, sz>(),
    mutex(),
    notEmpty(),
    notFull()
  {
  }


  template<typename MessageType, unsigned int sz>
  feabhOS_error MessageQueue<MessageType, sz>::post(const MessageType& in_msg, duration_mSec_t timeout)
  {
    CRITICAL_SECTION(mutex)
    {
      // Speculatively attempt to add to the buffer. If it's
      // full we'll get a FULL error message back.  If that's
      // the case, suspend until another task retrieves a
      // value.
      //
      while(FIFOBase::add(in_msg) == FIFOBase::FULL)
      {
        if(notFull.wait(mutex, timeout) != ERROR_OK)
        {
          return ERROR_TIMED_OUT;
        }
      }
    }
    // Wake any tasks waiting for data in the buffer.
    //
    notEmpty.notifyAll();
    return ERROR_OK;
  }


  template<typename MessageType, unsigned int sz>
  feabhOS_error MessageQueue<MessageType, sz>::get(MessageType& inout_msg, duration_mSec_t timeout)
  {
    CRITICAL_SECTION(mutex)
    {
      // NOTE: Here we must make a direct call
      // to the base class isEmpty() function,
      // rather than call our public method.  If
      // we call the public method we will deadlock
      // the system because we've already locked
      // the mutex!
      //
      while(FIFOBase::isEmpty())
      {
        if(notEmpty.wait(mutex, timeout) != ERROR_OK)
        {
          return ERROR_TIMED_OUT;
        }
      }
      FIFOBase::get(inout_msg);
    }
    // Wake any threads waiting for there to
    // be some space in the buffer.
    //
    notFull.notifyAll();
    return ERROR_OK;
  }


  template<typename MessageType, unsigned int sz>
  bool MessageQueue<MessageType, sz>::isEmpty()
  {
    CRITICAL_SECTION(mutex)
    {
      return FIFOBase::isEmpty();
    }
  }

}  // namespace FeabhOS


#endif // CPP03_FEABHOS_MESSAGEQUEUE_H
