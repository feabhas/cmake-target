// feabhOS_time_utils.c
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

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
