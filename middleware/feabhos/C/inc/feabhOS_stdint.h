// -------------------------------------------------------------------------------------
//  FeabhOS OS abstraction layer
//
//  DISCLAIMER:
//  Feabhas is furnishing this item "as is". Feabhas does not provide any warranty
//  of the item whatsoever, whether express, implied, or statutory, including, but
//  not limited to, any warranty of merchantability or fitness for a particular
//  purpose or any warranty that the contents of the item will be error-free.
//  In no respect shall Feabhas incur any liability for any damages, including, but
//  limited to, direct, indirect, special, or consequential damages arising out of,
//  resulting from, or any way connected to the use of the item, whether or not
//  based upon warranty, contract, tort, or otherwise; whether or not injury was
//  sustained by persons or property or otherwise; and whether or not loss was
//  sustained from, or arose out of, the results of, the item, or any services that
//  may be provided by Feabhas.
//
// -------------------------------------------------------------------------------------

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
