// feabhOS_time_utils.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef FEABHOS_TIME_UTILS_H
#define FEABHOS_TIME_UTILS_H

#include <time.h>
#include "feabhOS_time.h"

// -------------------------------------------------------------------------------------
// Create an absolute time from a duration.
// Please note, this function uses the current (system) time to
// generate the absolute duration, so it must be used immediately.
//
// Parameters:
// - period                This value will be added to the current
//                         time.
//
// Return values           current time + period
//
//
struct timespec abs_duration(duration_mSec_t period);


// -------------------------------------------------------------------------------------
// Convert the period into a struct timespec
//
// Parameters:
// - period                time duration in mSec
//
// Return values           duration as timespec
//
//
struct timespec to_timespec(duration_mSec_t period);



// -------------------------------------------------------------------------------------
// Add two timespec objects
//
// Parameters:
// - lhs, rhs              struct timespecs to add
//
// Return values           lhs + rhs
//
//
struct timespec timespec_add(const struct timespec* const lhs,
                             const struct timespec* const rhs);

#endif // FEABHOS_TIME_UTILS_H
