// feabhOS_time.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
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
