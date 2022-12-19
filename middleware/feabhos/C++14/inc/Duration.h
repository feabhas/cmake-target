// ----------------------------------------------------------------------------------
// Time.h
//
// DISCLAIMER:
// Feabhas is furnishing this item "as is". Feabhas does not provide any warranty
// of the item whatsoever, whether express, implied, or statutory, including, but
// not limited to, any warranty of merchantability or fitness for a particular
// purpose or any warranty that the contents of the item will be error-free.
// In no respect shall Feabhas incur any liability for any damages, including, but
// limited to, direct, indirect, special, or consequential damages arising out of,
// resulting from, or any way connected to the use of the item, whether or not
// based upon warranty, contract, tort, or otherwise; whether or not injury was
// sustained by persons or property or otherwise; and whether or not loss was
// sustained from, or arose out of, the results of, the item, or any services that
// may be provided by Feabhas.
// ----------------------------------------------------------------------------------

#ifndef CPP14_FEABHOS_DURATION_H
#define CPP14_FEABHOS_DURATION_H

#include "feabhOS_time.h"

namespace FeabhOS {

  namespace Time {

    struct Duration {
      unsigned long int value;
      constexpr operator duration_mSec_t() const           { return value; }
      constexpr bool operator==(const Duration& rhs) const { return this->value == rhs.value; }
      constexpr bool operator!=(const Duration& rhs) const { return this->value != rhs.value; }
      constexpr bool operator< (const Duration& rhs) const { return this->value <  rhs.value; }
      constexpr bool operator> (const Duration& rhs) const { return this->value >  rhs.value; }
    };


    inline constexpr Duration operator"" _ms(unsigned long long int t)
    {
      return Duration { static_cast<unsigned long>(t) };
    }

    // Constant values for blocking call timeouts
    //
    constexpr Time::Duration wait_forever { WAIT_FOREVER };
    constexpr Time::Duration no_wait      { NO_WAIT };

  } // namespace Time

} // namespace FeabhOS



#endif // CPP14_FEABHOS_DURATION_H
