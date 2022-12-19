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

#ifndef CPP14_FEABHOS_EVENTGROUP_H
#define CPP14_FEABHOS_EVENTGROUP_H

#include <stdexcept>
#include "Condition.h"
#include "Mutex.h"
#include "Duration.h"
#include "Bitops.h"

using FeabhOS::Utility::bit;
using FeabhOS::Utility::bit_range;
using FeabhOS::Utility::is_set;

namespace FeabhOS {

  // -------------------------------------------------
  // Event Group exceptions
  //
  class invalid_flag : public std::out_of_range {
  public:
    invalid_flag(const char* str) : std::out_of_range(str) {}
    invalid_flag() : std::out_of_range("Invalid flag index") {}
  };


  // -------------------------------------------------
  // As the number of flags need not be a multiple of
  // 8, this compile-time function rounds up to the nearest
  // word length.
  //
  constexpr inline std::size_t round_up(unsigned int num_bits)
  {
    if (num_bits <= 8)  return 8;
    if (num_bits <= 16) return 16;
    if (num_bits <= 32) return 32;
    return 0;
  }

  // -------------------------------------------------
  // Traits class for determining the underlying
  // type used to store the event flags.  Note there
  // is a maximum of 32 flags per event group.
  //
  template <std::size_t num_bits>
  struct EventGroupTraits {  };

  template <>
  struct EventGroupTraits<8> { using type = std::uint8_t; };

  template <>
  struct EventGroupTraits<16> { using type = std::uint16_t; };

  template <>
  struct EventGroupTraits<32> { using type = std::uint32_t; };


  // -------------------------------------------------
  // An EventGroup represents a set of N event flags.
  // The EventGroup allows either conjunctive or
  // disjunctive waiting.  For more details on the
  // event flag semantics review the C API header
  //
  template <std::size_t num_flags>
  class EventGroup {
  public:
    using Bitmask_Ty = typename EventGroupTraits<round_up(num_flags)>::type;

    EventGroup() = default;

    // XXX = any       - Disjunctive wait
    // XXX = all       - Conjunctive wait
    //
    // wait_XXX()      - Blocking call; will block forever on
    //                   bit pattern
    //
    // try_wait_XXX()  - Non-blocking; will return false if pattern
    //                   match fails
    //
    // wait_XXX_for()  - Blocking call; will wait for block until
    //                   timeout expires
    //
    void wait_any    (Bitmask_Ty flags_to_check);
    bool try_wait_any(Bitmask_Ty flags_to_check);
    bool wait_any_for(Bitmask_Ty flags_to_check, const Time::Duration& timeout);

    void wait_all    (Bitmask_Ty flags_to_check);
    bool try_wait_all(Bitmask_Ty flags_to_check);
    bool wait_all_for(Bitmask_Ty flags_to_check, const Time::Duration& timeout);

    // Signal one or more event flags
    //
    void set(Bitmask_Ty bits_to_set);

    // Read current state of event group
    //
    operator Bitmask_Ty() const;

    // Reset any signalled event flags
    //
    void clear(Bitmask_Ty bits_to_clear);
    void clear_all();

    // ----------------------------------------
    // Proxy for setting/clearing individual
    // event flags.
    //
    class Flag {
    public:
      // Signal / clear the flag
      //
      inline Flag& operator=(int val);
      inline Flag& operator=(const Flag& rhs);

      // Return the flag's state
      //
      inline operator int() const;

    private:
      friend class EventGroup;
      Flag(EventGroup& owner, unsigned num) : parent(&owner), flag(num) { }

      EventGroup* parent;
      unsigned    flag;
    };

    // Return the state of a single event flag
    //
    inline int  operator[](unsigned int flag) const;
    inline Flag operator[](unsigned int flag);

    // Copy / move policy
    //
    EventGroup(const EventGroup&)     = delete;
    void operator=(const EventGroup&) = delete;
    EventGroup(EventGroup&&)          = delete;
    void operator=(EventGroup&&)      = delete;

  private:
    Bitmask_Ty state    { };
    Mutex     mutex    { };
    Condition flag_set { };
  };


  template <std::size_t num_flags>
  void EventGroup<num_flags>::set(EventGroup<num_flags>::Bitmask_Ty bits_to_set)
  {
    CRITICAL_SECTION(mutex)
    {
      state |= bits_to_set;
      flag_set.notify_all();
    }
  }


  template <std::size_t num_flags>
  void EventGroup<num_flags>::clear(EventGroup<num_flags>::Bitmask_Ty bits_to_clear)
  {
    CRITICAL_SECTION(mutex)
    {
      state &= ~(bits_to_clear);
    }
  }


  template <std::size_t num_flags>
  void EventGroup<num_flags>::clear_all()
  {
    clear(bit_range(0, num_flags));
  }


  template <std::size_t num_flags>
  int EventGroup<num_flags>::operator[](unsigned int flag) const
  {
    if ((flag < 0) || (flag >= num_flags)) throw invalid_flag { };
    return is_set(state, flag) ? 1 : 0;
  }


  template <std::size_t num_flags>
  typename EventGroup<num_flags>::Flag
  EventGroup<num_flags>::operator[](unsigned int flag)
  {
    if (flag >= num_flags) throw invalid_flag { };
    return Flag { *this, flag };
  }


  template <std::size_t num_flags>
  EventGroup<num_flags>::operator EventGroup<num_flags>::Bitmask_Ty() const
  {
    return state;
  }


  template <std::size_t num_flags>
  void EventGroup<num_flags>::wait_any(EventGroup<num_flags>::Bitmask_Ty flags_to_check)
  {
    wait_any_for(flags_to_check, Time::wait_forever);
  }


  template <std::size_t num_flags>
  bool EventGroup<num_flags>::try_wait_any(EventGroup<num_flags>::Bitmask_Ty flags_to_check)
  {
    return (wait_any_for(flags_to_check, Time::no_wait) == ERROR_OK);
  }


  template <std::size_t num_flags>
  bool EventGroup<num_flags>::wait_any_for(EventGroup<num_flags>::Bitmask_Ty flags_to_check,
                                           const Time::Duration& timeout)
  {
    flags_to_check &= bit_range(0, num_flags);

    CRITICAL_SECTION(mutex)
    {
      // AND the current bit pattern with the bits to check.
      // If any match this will yield a non-zero result.
      //
      while((state & flags_to_check) == 0) {

        bool success = flag_set.wait_for(mutex, timeout);
        if (!success) return false;
      }
    }
    return true;
  }


  template <std::size_t num_flags>
  void EventGroup<num_flags>::wait_all(EventGroup<num_flags>::Bitmask_Ty flags_to_check)
  {
    wait_all_for(flags_to_check, Time::wait_forever);
  }


  template <std::size_t num_flags>
  bool EventGroup<num_flags>::try_wait_all(EventGroup<num_flags>::Bitmask_Ty flags_to_check)
  {
    return (wait_all_for(flags_to_check, Time::no_wait) == ERROR_OK);
  }


  template <std::size_t num_flags>
  bool EventGroup<num_flags>::wait_all_for(EventGroup<num_flags>::Bitmask_Ty flags_to_check,
                                           const Time::Duration& timeout)
  {
    flags_to_check &= bit_range(0, num_flags);

    CRITICAL_SECTION(mutex)
    {
      while ((state & flags_to_check) != flags_to_check) {

        bool success = flag_set.wait_for(mutex, timeout);
        if (!success) return false;
      }
    }
    return true;
  }


  template <std::size_t num_flags>
  typename EventGroup<num_flags>::Flag&
  EventGroup<num_flags>::Flag::operator=(int val)
  {
   if (val == 0) parent->clear(bit(flag));
   else          parent->set  (bit(flag));
   return *this;
  }


  template <std::size_t num_flags>
  EventGroup<num_flags>::Flag::operator int() const
  {
    return is_set(parent->state, flag) ? 1 : 0;
  }


  template <std::size_t num_flags>
  typename EventGroup<num_flags>::Flag&
  EventGroup<num_flags>::Flag::operator=(const Flag& rhs)
  {
    *this = static_cast<int>(rhs);
    return *this;
  }

} // namespace FeabhOS

#endif // CPP03_FEABHOS_EVENTGROUP_H

