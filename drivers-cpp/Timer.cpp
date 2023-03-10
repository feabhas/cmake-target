// -----------------------------------------------------------------------------
// Timer.cpp
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

#include "Timer.h"

#ifdef RTOS

extern "C" 
void vApplicationTickHook(void)
{
  // only required for C++ timer
}

// Timer functions for non-RTOS operation --------------------------------------- 
//
#else   

#include "cmsis_device.h"

#define TIMER_FREQUENCY_HZ (1000u)

#include <atomic>

namespace {
// Timer state
//
  std::atomic<uint32_t> timer_counter {};
  bool timer_started{};

  void start_timer()
  {
    SysTick_Config(SystemCoreClock / TIMER_FREQUENCY_HZ);
    timer_started = true;
  }

}

void sleep(duration_mSec period)
{
  sleep(std::chrono::milliseconds(period));
}

void sleep(std::chrono::milliseconds period)
{
  if(!timer_started) {
    start_timer();
  }
  
  timer_counter = std::uint32_t(period.count());

  while(timer_counter != 0){
    // Wait for timer to count down...
    __WFE();
  }
}


extern "C" void SysTick_Handler (void)
{
  if(timer_counter != 0){
    --timer_counter;
  }
}

#endif 
