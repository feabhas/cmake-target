// usart_buffer.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef USART_BUFFER_H
#define USART_BUFFER_H

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
