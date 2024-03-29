// EventGroup.cpp
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#include "EventGroup.h"

namespace FeabhOS
{
  EventGroup::EventGroup() :
    handle(0),
    state(0)
  {
    feabhOS_eventflags_create(&handle);
  }


  EventGroup::~EventGroup()
  {
    feabhOS_eventflags_destroy(&handle);
  }


  void EventGroup::set(bitmask8_t bits_to_set)
  {
    feabhOS_eventflags_set(&handle, bits_to_set);
  }


  void EventGroup::clear(bitmask8_t bits_to_clear)
  {
    feabhOS_eventflags_clear(&handle, bits_to_clear);
  }


  void EventGroup::clearAll()
  {
    feabhOS_eventflags_clear_all(&handle);
  }


  int EventGroup::operator[](int flag) const
  {
    if((flag < 0) || (flag >= num_flags)) throw InvalidFlag();

    return ((state & (1 << flag)) != 0) ? 1 : 0;
  }


  EventGroup::Flag EventGroup::operator[](int flag)
  {
    if((flag < 0) || (flag >= num_flags)) throw InvalidFlag();
    return Flag(*this, flag);
  }

  EventGroup::operator bitmask8_t() const
  {
    return state;
  }


  feabhOS_error EventGroup::wait_any(bitmask8_t flags_to_check, duration_mSec_t timeout)
  {
    state = flags_to_check;
    return feabhOS_eventflags_wait_any(&handle, &state, timeout);
  }


  feabhOS_error EventGroup::wait_all(bitmask8_t flags_to_check, duration_mSec_t timeout)
  {
    state = flags_to_check;
    return feabhOS_eventflags_wait_all(&handle, &state, timeout);
  }


  EventGroup::Flag& EventGroup::Flag::operator=(int val)
  {
   if(val == 0) parent->clear(1 << flag);
   else         parent->set(1 << flag);
   return *this;
  }


  EventGroup::Flag::operator int() const
  {
    return ((parent->state & (1 << flag)) != 0) ? 1 : 0;
  }


  EventGroup::Flag& EventGroup::Flag::operator=(const Flag& rhs)
  {
    *this = static_cast<int>(rhs);
    return *this;
  }

} // namespace FeabhOS



