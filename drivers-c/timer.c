// -----------------------------------------------------------------------------
// timer.c
//
// DISCLAIMER:
// Feabhas is furnishing this item "as is". Feabhas does not provide any
// warranty of the item whatsoever, whether express, implied, or statutory,
// including, but not limited to, any warranty of merchantability or fitness
// for a particular purpose or any warranty that the contents of the item will
// be error-free.
// In no respect shall Feabhas incur any liability for any damages, including,
// but limited to, direct, indirect, special, or consequential damages arising
// out of, resulting from, or any way connected to the use of the item, whether
// or not based upon warranty, contract, tort, or otherwise; whether or not
// injury was sustained by persons or property or otherwise; and whether or not
// loss was sustained from, or arose out of, the results of, the item, or any
// services that may be provided by Feabhas.
// -----------------------------------------------------------------------------

// Timer functions for RTOS operation ------------------------------------------ 
//
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




