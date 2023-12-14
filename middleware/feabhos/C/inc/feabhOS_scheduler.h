// feabhOS_scheduler.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef FEABHOS_SCHEDULER_H
#define FEABHOS_SCHEDULER_H

#include "feabhOS_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

feabhOS_error feabhOS_scheduler_init(void);
feabhOS_error feabhOS_scheduler_start(void);


#ifdef __cplusplus
}
#endif

#endif /* FEABHOS_SCHEDULER_H */

