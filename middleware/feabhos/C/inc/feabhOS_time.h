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

#ifndef FEABHOS_TIME_H
#define FEABHOS_TIME_H

#include <feabhOS_port_defs.h>
#include <stdint.h>

// Type aliases for time.
// A duration represents a period of time
// An observation represents an absolute time
//
typedef uint32_t duration_mSec_t;
typedef uint32_t observation_mSec_t;

//  Special time durations for use with
//  blocking OS calls (for example, waiting on
//  a mutex)
//
//  NO_WAIT is equivalent to waiting for a zero duration
//  WAIT_FOREVER means block until signalled (that is, never time out)
//
#define NO_WAIT         (duration_mSec_t)OS_ZERO_TIMEOUT
#define WAIT_FOREVER    (duration_mSec_t)OS_INFINITE_TIMEOUT

#endif /* FEABHOS_TIME_H */
