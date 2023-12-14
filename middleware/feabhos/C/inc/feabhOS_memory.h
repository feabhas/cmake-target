// feabhOS_memory.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef FEABHOS_MEMORY_H
#define FEABHOS_MEMORY_H

#include "feabhOS_errors.h"
#include "feabhOS_stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

void  feabhOS_memory_init(void);
void *feabhOS_memory_alloc(size_bytes_t sz);
void  feabhOS_memory_free(void * const pMemory);

#ifdef __cplusplus
}
#endif

#endif /* FEABHOS_MEMORY_H */
