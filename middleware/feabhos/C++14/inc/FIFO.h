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

#ifndef CPP14_FEABHOS_FIFO_H
#define CPP14_FEABHOS_FIFO_H

#include <cstddef>
#include <array>

// -------------------------------------------------------------------------------------
// A basic circular buffer used by message queues
// -------------------------------------------------------------------------------------

namespace FeabhOS {

  namespace Utility {

    template <typename T = int, std::size_t sz = 8>
    class FIFO {
    public:
      enum Error { OK, FULL, EMPTY };

      template <typename U>
      Error add(U&& in_val);

      Error get(T& inout_val);

      bool        is_empty() const { return (num_items == 0); }
      std::size_t size()     const { return num_items; }
      std::size_t capacity() const { return sz; }

    private:
      using FIFO_Ty     = std::array<T, sz>;
      using Iterator_Ty = typename FIFO_Ty::iterator;

      FIFO_Ty     buffer    { };
      Iterator_Ty read      { std::begin(buffer) };
      Iterator_Ty write     { std::begin(buffer) };
      std::size_t num_items { 0 };
    };


    template <typename T, std::size_t sz>
    template <typename U>
    typename FIFO<T, sz>::Error
    FIFO<T, sz>::add(U&& in_val)
    {
      if (num_items == sz) return FULL;

      *write = std::forward<U>(in_val);
      ++num_items;
      ++write;
      if (write == std::end(buffer)) write = std::begin(buffer);

      return OK;
    }


    template <typename T, size_t sz>
    typename FIFO<T, sz>::Error 
    FIFO<T, sz>::get(T& inout_val)
    {
      if (num_items == 0) return EMPTY;

      inout_val = std::move(*read);
      --num_items;
      ++read;
      if (read == std::end(buffer)) read = std::begin(buffer);

      return OK;
    }

  }  // namespace Utility

} // namespace FeabhOS

#endif // CPP14_FEABHOS_FIFO_H
