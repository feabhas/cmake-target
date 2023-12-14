// feabhOS_stdint.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef FEABHOS_STDINT_H
#define FEABHOS_STDINT_H


// Base types for everything else
//
#include <stdint.h>


// Specifying the size of things
//
typedef uint32_t num_elements_t;
typedef uint32_t size_bytes_t;


// For use when specifying bit patterns
//
typedef uint8_t  bitmask8_t;
typedef uint16_t bitmask16_t;
typedef uint32_t bitmask32_t;


#endif /* FEABHOS_STDINT_H */
