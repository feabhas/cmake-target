// -----------------------------------------------------------------------------
// peripherals.h
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
#ifndef PERIPHERALS_H_
#define PERIPHERALS_H_

namespace STM32F407
{
    enum AHB1_Device
    {
      GPIO_A    = 0,
      GPIO_B    = 1,
      GPIO_C    = 2,
      GPIO_D    = 3,
      GPIO_E    = 4,
      GPIO_F    = 5,
      GPIO_G    = 6,
      GPIO_H    = 7,
      GPIO_I    = 8
    };

    enum APB1_Device
    {
      TIMER_2   = 0,
      TIMER_3   = 1,
      TIMER_4   = 2,
      TIMER_5   = 3,
      TIMER_6   = 4,
      TIMER_7   = 5,
      TIMER_12  = 6,
      TIMER_13  = 7,
      TIMER_14  = 8,
      SPI_2     = 14,
      SPI_3     = 15,
      USART_2   = 17,
      USART_3   = 18,
      USART_4   = 19,
      USART_5   = 20,
      I2C_1     = 21,
      I2C_2     = 22,
      I2C_3     = 23,
    };

    enum APB2_Device
    {
      TIMER_1   = 0,
      TIMER_8   = 1,
      USART_1   = 4,
      USART_6   = 5,
      ADC_1     = 8,
      ADC_2     = 9,
      ADC_3     = 10,
      TIMER_9   = 16,
      TIMER_10  = 17,
      TIMER_11  = 18,
    };

    void enable(AHB1_Device device);
    void enable(APB1_Device device);
    void enable(APB2_Device device);

    void disable(AHB1_Device device);
    void disable(APB1_Device device);
    void disable(APB2_Device device);

} // namespace STM32F407

#endif /* PERIPHERALS_H_ */
