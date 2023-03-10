// -----------------------------------------------------------------------------
// usart_utils.c
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

#include "stdint.h"
#include "stm32f4xx.h"
#include "usart_utils.h"


// USART3 GPIO Configuration
// PortB:10     ------> USART3_TX
// PortB:11     ------> USART3_RX
//
void usart_enable_IO(void)
{
  // Enable GPIO B IO Port Clock
  uint32_t ahb1enr = RCC->AHB1ENR;
  ahb1enr |= (0x1u << 1);			// Bit1 : GPIOBEN
  RCC->AHB1ENR = ahb1enr;

  // if pinNum > 8 then use AFR[1]
  // 4 bits per pin
  // AF7 (0x07) sets the GPIO B (to USART3 AF functions)

  uint32_t afr1 = GPIOB->AFR[1];
  afr1  |= (0x07 << ( 4 * (10 % 8)));	// USART 3 Alt fn
  afr1  |= (0x07 << ( 4 * (11 % 8)));	// 
  GPIOB->AFR[1] = afr1;

  uint32_t moder = GPIOB->MODER;
  moder  |= (0x02 << (10 * 2));		      // Alt function mode
  moder  |= (0x02 << (11 * 2));		      // 
  GPIOB->MODER= moder;

  uint32_t ospeedr = GPIOB->OSPEEDR;
  ospeedr |= (0x03 << (10 * 2));	         // High speed
  ospeedr |= (0x03 << (11 * 2));	         // 
  GPIOB->OSPEEDR = ospeedr;

  uint32_t pupdr = GPIOB->PUPDR;
  pupdr  |= (0x01 << (10 * 2));		       // Pull-up
  pupdr  |= (0x01 << (11 * 2));		       // 
  GPIOB->PUPDR = pupdr;
}

void usart_utils_init(void)
{
    NVIC_DisableIRQ(USART3_IRQn);
    usart_enable_IO();

      // Enable GPIO B IO Port Clock
    uint32_t apb1enr = RCC->APB1ENR;
    apb1enr |= (0x1u << 18);                // Bit 18 : USART3EN
    RCC->APB1ENR = apb1enr;


    uint32_t cr1 = USART3->CR1;
    cr1 &= ~((0x1u << 12) | (0x1u << 10));  // no parity, 1 start, 8 data
    USART3->CR1 =  cr1;

    uint32_t cr2 = USART3->CR2;
    cr2 &= ~(0x3u << 12);                   // 1 stop bit
    USART3->CR2 = cr2;

    uint32_t brr = USART3->BRR;
    brr = 0x008Bu;                          // Baud_rate::baud_115200
    USART3->BRR = brr;

    cr1 = USART3->CR1;
    cr1 |= (0x1u << 13)  | (0x1u << 3) | (0x1u << 2);   // UE, TE, RE
    USART3->CR1 = cr1;
}

void usart_utils_enable_rx_interrupts(void)
{
    NVIC_SetPriority(USART3_IRQn, 10);
    uint32_t cr1 = USART3->CR1;
    cr1 |= (0x1u << 5);                   // RXNEIE
    USART3->CR1 =  cr1;
}
