// feabhOS_memory.c
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#include <stdlib.h>
#include "feabhOS_memory.h"



void feabhOS_memory_init(void)
{
	// historical
}


void *feabhOS_memory_alloc(size_bytes_t sz)
{
  return malloc(sz);
}


void  feabhOS_memory_free(void * const pMemory)
{
	free(pMemory);
}
