// Callback.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef CALLBACK_H
#define CALLBACK_H

#include <functional>
#include <algorithm>

namespace FeabhOS {

  namespace Utility {

    // ------------------------------------------------------------------------------
    // Callback represents a generic callback object.
    // Note:
    // The parameters for fn are stored as a tuple inside the
    // Callback object.  Since fn (probably) doesn't take a
    // tuple as a parameter we have to 'unpack' the tuple into
    // individual parameters.  This is done using the private
    // helper function call_with_tuple().  The behaviour happens
    // as follows:
    // - std::index_sequence_for() is a constant-expression function
    //   that generates an integer (size_t) number sequence - std::index_sequence
    //   (starting at 0)for each parameter in Param_Ty.
    //
    // - call_with_tuple() unpacks the std::index_sequence variadic template to
    //   call get<> on each member of the params tuple.
    //
    template<typename Fn_Ty, typename ... Param_Ty>
    class Callback {
    public:
      Callback(Fn_Ty func, Param_Ty&&... arg) :
        fn     { func },
        params { std::forward<Param_Ty>(arg)... }
      {
      }


      void operator()()
      {
        call_with_tuple(std::index_sequence_for<Param_Ty...>());
      }

    private:

      template<std::size_t ... Index>
      void call_with_tuple(std::index_sequence<Index...>)
      {
        fn(std::get<Index>(params)...);
      }

      Fn_Ty fn;
      std::tuple<Param_Ty...> params;
    };

  } // namespace Utility

} // namespace FeabhOS

#endif // CALLBACK_H
