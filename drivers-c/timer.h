// timer.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef TIMER_H
#define TIMER_H

typedef unsigned long duration_mSec;

#ifdef RTOS
#include "feabhOS_task.h"
#define sleep feabhOS_task_sleep
#else
void sleep(duration_mSec period);
#endif

#endif // TIMER_H_
