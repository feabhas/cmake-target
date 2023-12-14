// feabhOS_allocator.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef FEABHOS_C_FREERTOS_INC_FEABHOS_ALLOCATOR_H
#define FEABHOS_C_FREERTOS_INC_FEABHOS_ALLOCATOR_H

#include "feabhOS_stdint.h"
#include "feabhOS_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------------------------
//  This macro defines the maximum number of pools
//  available.  If you are using static allocation
//  (that is, a fixed number) for feabhOS constructs
//  - tasks, etc. - you should ensure there is a pool
//  available for each of them.  If there are not
//  enough pools available your program will assert.
//
#define MAX_POOLS 8

// -----------------------------------------------------------------------------------------------
// The pool handle.  You must create a handle for
// each pool you want to use.
//
typedef struct feabhOS_pool* feabhOS_POOL;


// -----------------------------------------------------------------------------------------------
// Create a fixed-block allocator
// Initialise a fixed-block allocator in user-supplied memory.
// Parameters:
// - pool_handler          A pointer to a feabhOS_POOL
// - pool_memory           A pointer to a region of static memory
//                         where the allocator will be constructed
// - pool_size             The size (in bytes) of the static memory
// - block_size            The required size of each block in the pool
// - num_blocks            The number of blocks
//
// Return values
// ERROR_OK                Success.  Pool handle will be non-NULL
// ERROR_MEMORY            Could not allocate memory for the pool
//                         management structures.
// ERROR_PARAM1            pool_memory is NULL
// ERROR_PARAM2            pool_size < (block_size * num_blocks)
// ERROR_PARAM3            block_size == 0
//                         block_size < sizeof(uintptr_t)
// ERROR_PARAM4            num_blocks == 0
//
feabhOS_error feabhOS_pool_create(feabhOS_POOL * const pool_handle,
                                  void*                pool_memory,
                                  size_bytes_t         pool_size,
                                  size_bytes_t         block_size,
                                  num_elements_t       num_blocks);

// -----------------------------------------------------------------------------------------------
// Allocate a block
// Allocate a block from the specified pool.
// Parameters:
// - pool_handle          A pointer to a feabhOS_POOL
//
// Return values
// NULL if the memory cannot be allocated.
//
void* feabhOS_block_allocate(feabhOS_POOL * const pool_handle);


// -----------------------------------------------------------------------------------------------
// Free a block
// Return a block to the specified pool.
// Freeing a NULL pointer has no effect.
// Parameters:
// - pool_handle          A pointer to a feabhOS_POOL
// - block                A pointer to the block to be freed.
//
// Return values
// ERROR_OK               The block was successfully freed
// ERROR_INVALID_HANDLE   The pool handle was NULL
// ERROR_STUPID           The block was not allocated from
//                        the specified pool
//
feabhOS_error feabhOS_block_free(feabhOS_POOL * const pool_handle, void * block);


#ifdef __cplusplus
}
#endif

#endif // FEABHOS_C_FREERTOS_INC_FEABHOS_ALLOCATOR_H_
