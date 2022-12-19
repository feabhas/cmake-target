// -----------------------------------------------------------------------------
// usart_buffer.c
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

#include "usart_buffer.h"

static uint32_t buffer[USART_BUFFER_SIZE];     // Array of void values   
static unsigned int next_in   = 0;        // Next input location      
static unsigned int next_out  = 0;        // Next output location      
static unsigned int num_items = 0;        // Current buffer size 


void usart_buffer_init(void)
{
}


Usart_Buffer_error usart_buffer_add(uint32_t to_store)
{
  if(num_items == USART_BUFFER_SIZE) return USART_BUFFER_FULL;

  buffer[next_in] = to_store;
  ++next_in;
  ++num_items;
  if(next_in == USART_BUFFER_SIZE) next_in = 0;

  return USART_BUFFER_OK;
}


Usart_Buffer_error usart_buffer_get(uint32_t * const value)
{
  if(num_items == 0) return USART_BUFFER_EMPTY;

  *value = buffer[next_out];
  ++next_out;
  --num_items;
  if(next_out == USART_BUFFER_SIZE) next_out = 0;

  return USART_BUFFER_OK;
}


bool usart_buffer_is_empty(void)
{
  return (num_items == 0);
}
