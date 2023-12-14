// feabhOS_interrupts.c
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#include <assert.h>
#include <stdbool.h>
#include "feabhOS_interrupts.h"


void feabhOS_enable_interrupts(void)
{
  // Interrupt management is not
  // available in user space code.
  //
  assert(false);
}


void feabhOS_disable_interrupts(void)
{
  // Interrupt management is not
  // available in user space code.
  //
  assert(false);
}
