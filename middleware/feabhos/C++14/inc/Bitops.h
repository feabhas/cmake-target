// Bitops.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef CPP14_FEABHOS_BITOPS_H
#define CPP14_FEABHOS_BITOPS_H

namespace FeabhOS {

  namespace Utility {

    // Creates a uint32_t where bit n is set
    //
    inline constexpr
    uint32_t bit(uint32_t n)
    {
      return (1 << n);
    }


    // Creates a uint32_t where bits start -> end
    // (inclusive) are set
    //
    inline constexpr
    uint32_t bit_range(uint32_t start, uint32_t end)
    {
      uint32_t value = 0;
      for(uint32_t i = start; i <= end; ++i)
      {
        value |= bit(i);
      }
      return value;
    }


    // Returns true if bit b is set in word
    //
    inline constexpr
    bool is_set(uint32_t word, uint32_t b)
    {
      return ((word & bit(b)) != 0);
    }
  }
}

#endif // CPP14_FEABHOS_BITOPS_H
