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

#ifndef CPP03_FEABHOS_EVENTGROUP_H
#define CPP03_FEABHOS_EVENTGROUP_H

#include <stdexcept>
#include "feabhOS_eventflags.h"

namespace FeabhOS
{
  // -------------------------------------------------
  // Event Group exceptions
  //
  class InvalidFlag : public std::out_of_range
  {
  public:
    InvalidFlag(const char* str) : std::out_of_range(str) {}
    InvalidFlag() : std::out_of_range("Invalid flag index") {}
  };

  // -------------------------------------------------
  // An EventGroup represents a set of 8 event flags.
  // The EventGroup allows either conjunctive or
  // disjunctive waiting.  For more details on the
  // event flag semantics review the C API header
  //
  class EventGroup
  {
  public:
    EventGroup();
    ~EventGroup();

    // Pend on a group of flags
    //
    feabhOS_error wait_any(bitmask8_t flags_to_check, duration_mSec_t timeout);
    feabhOS_error wait_all(bitmask8_t flags_to_check, duration_mSec_t timeout);

    // Signal one or more event flags
    //
    void set(bitmask8_t bits_to_set);

    // Read current state of event group
    //
    operator bitmask8_t() const;

    // Reset any signalled event flags
    //
    void clear(bitmask8_t bits_to_clear);
    void clearAll();

    // ----------------------------------------
    // Proxy for setting/clearing individual
    // event flags
    //
    class Flag
    {
    public:
      // Signal / clear the flag
      //
      Flag& operator=(int val);
      Flag& operator=(const Flag& rhs);

      // Return the flag's state
      //
      operator int() const;

    private:
      friend class EventGroup;
      Flag(EventGroup& owner, int num) : parent(&owner), flag(num) { }

      EventGroup* parent;
      int         flag;
    };

    // Return the state of a single event flag
    //
    int  operator[](int flag) const;
    Flag operator[](int flag);

  private:
    EventGroup(const EventGroup&);
    void operator=(const EventGroup&);

    static const int num_flags = 8;

    feabhOS_EVENTFLAGS handle;
    bitmask8_t state;
  };

} // namespace FeabhOS

#endif // CPP03_FEABHOS_EVENTGROUP_H

