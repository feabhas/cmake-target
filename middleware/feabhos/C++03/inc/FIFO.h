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

#ifndef CPP03_FEABHOS_FIFO_H
#define CPP03_FEABHOS_FIFO_H

#include <cstdint>


namespace FeabhOS
{
  namespace Utility
  {
    template <typename T = int, std::size_t sz = 8>
    class FIFO
    {
    public:
      enum Error { OK, FULL, EMPTY };

      FIFO();
      ~FIFO();

      Error add(T value);
      Error get(T& value);
      bool isEmpty();
      void flush();

    private:
      unsigned int read;
      unsigned int write;
      unsigned int numItems;

      T buffer[sz];
    };


    template <typename T, std::size_t sz>
    FIFO<T, sz>::FIFO() : buffer()
    {
      flush();
    }


    template <typename T, std::size_t sz>
    FIFO<T, sz>::~FIFO()
    {
    }

  
    template <typename T, std::size_t sz>
    typename FIFO<T, sz>::Error FIFO<T, sz>::add(T value)
    {
      if(numItems == sz)
      {
        return FULL;
      }
      else
      {
        buffer[write] = value;

        ++numItems;
        ++write;
        if(write == sz) write = 0;

        return OK;
      }
    }


    template <typename T, std::size_t sz>
    typename FIFO<T, sz>::Error FIFO<T, sz>::get(T& value)
    {
      if(numItems == 0)
      {
        return EMPTY;
      }
      else
      {
        value = buffer[read];

        --numItems;
        ++read;
        if(read == sz) read = 0;

        return OK;
      }
    }


    template <typename T, std::size_t sz>
    bool  FIFO<T, sz>::isEmpty()
    {
      return (numItems == 0);
    }


    template <typename T, std::size_t sz>
    void  FIFO<T, sz>::flush()
    {
      read = 0;
      write = 0;
      numItems = 0;
    }

  } // namespace Utility
}   // namespace FeabhOS

#endif // CPP03_FEABHOS_FIFO_H
