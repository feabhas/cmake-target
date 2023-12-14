// Timer.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef TIMER_H
#define TIMER_H

#ifdef RTOS
#include "feabhOS_task.h"
#define sleep feabhOS_task_sleep
#else

#include <chrono>
using namespace std::chrono_literals;
void sleep(std::chrono::milliseconds delay);

using duration_mSec = unsigned long ;
// [[deprecated("Use sleep(1000ms) instead")]]
void sleep(duration_mSec period);

#endif

#endif // TIMER_H_
