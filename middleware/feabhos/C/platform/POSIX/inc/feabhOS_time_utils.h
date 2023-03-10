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
