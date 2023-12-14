// Duration.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
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
