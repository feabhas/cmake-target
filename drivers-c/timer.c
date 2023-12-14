// timer.c
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#include "timer.h"
#ifdef RTOS

void vApplicationTickHook(void)
{
  // only required for C++ timer
}

// Timer functions for non-RTOS operation ---------------------------------------
//
#else

#include <stdbool.h>
#include "cmsis_device.h"

#define TIMER_FREQUENCY_HZ (1000u)


// Timer state
//
static volatile duration_mSec timer_counter = 0;
static bool timer_started = false;

static void start_timer(void)
{
  SysTick_Config(SystemCoreClock / TIMER_FREQUENCY_HZ);
  timer_started = true;
}

void sleep(duration_mSec period)
{
  if(!timer_started) {
    start_timer();
  }

  timer_counter = period;

  while(timer_counter != 0)
  {
    // Wait for timer to count down...
    __WFE();
  }
}


void SysTick_Handler (void)
{
  if(timer_counter != 0)
  {
    --timer_counter;
  }
}

#endif




