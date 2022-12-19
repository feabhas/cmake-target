// -----------------------------------------------------------------------------
// usart_buffer.h
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

#pragma once
#ifndef USART_BUFFER_H_
#define USART_BUFFER_H_

#include <stdbool.h>
#include <stdint.h>

// Buffer configuration
//
#define USART_BUFFER_SIZE   4


// Buffer error codes 
//
typedef enum
{
  USART_BUFFER_OK,
  USART_BUFFER_FULL,
  USART_BUFFER_EMPTY
} Usart_Buffer_error;


// Initialisation should be performed before the buffer is used.  
// The initialisation function sets up the buffer and ensures it is ready 
// to receive / retrieve data.
//
void usart_buffer_init(void);


// The value to be added is passed as a parameter.  
// The value is stored in the buffer at the current free location.  
// If the buffer is full the value will not be stored and an error value
// will be returned to the caller.
//
Usart_Buffer_error usart_buffer_add(uint32_t to_store);


// The value at the current 'head' of the buffer is returned.  
// If the buffer is empty the return value pointer will be NULL and 
// an error value is returned to the caller.
//
Usart_Buffer_error usart_buffer_get(uint32_t * const value);


// Returns whether the buffer is currently
// empty.
//
bool usart_buffer_is_empty(void);

#endif
