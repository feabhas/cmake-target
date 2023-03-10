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

#include "feabhOS_time_utils.h"

// ----------------------------------------------------------------------------
//
struct timespec abs_duration(duration_mSec_t period)
{
  struct timespec current_time;
  clock_gettime(CLOCK_REALTIME, &current_time);

  struct timespec offset = to_timespec(period);

  return timespec_add(&current_time, &offset);
}


// ----------------------------------------------------------------------------
//
struct timespec to_timespec(duration_mSec_t period)
{
  time_t period_sec  = period / 1000;
  long   period_nsec = (period - (period_sec * 1000)) * 1000000L;

  struct timespec duration =
  {
    .tv_sec  = period_sec,
    .tv_nsec = period_nsec
  };

  return duration;
}


// ----------------------------------------------------------------------------
//
struct timespec timespec_add(const struct timespec* const lhs,
                             const struct timespec* const rhs)
{
  struct timespec result;

  result.tv_sec  = lhs->tv_sec + rhs->tv_sec;
  result.tv_nsec = (lhs->tv_nsec + rhs->tv_nsec) % 1000000000L;

  return result;
}
