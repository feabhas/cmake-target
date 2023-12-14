// Timer.cpp
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

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
