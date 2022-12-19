// -----------------------------------------------------------------------------
// USART.cpp
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

#include "USART.h"
#include <cstdint>
#include "Peripherals.h"
#include "Memory_map.h"
#include "USART_utils.h"

namespace STM32F407
{
  // UART register definitions
  //
  struct USART_registers
  {
    std::uint32_t STATUS;         // Status register
    std::uint32_t DATA;           // Data register
    std::uint32_t BAUD_RATE;      // Baud rate register
    std::uint32_t CTRL_1;         // Control register 1
    std::uint32_t CTRL_2;         // Control register 2
    std::uint32_t CTRL_3;         // Control register 3
    std::uint32_t GUARD_PRESCALE; // Guard time and prescaler register
  };

  // For this project, there is only one UART
  // - USART 3
  //
  static constexpr std::uint32_t uart_addr { 0x40004800 };


  USART::USART() :
    usart { reinterpret_cast<USART_registers*>(uart_addr) }
  {
    // Enable the USART clock.
    //
    STM32F407::enable(STM32F407::USART_3);

    // Stop the USART before configuring
    //
    disable();

    // Reset STOP bits
    //
    std::uint32_t ctrl_2 = usart->CTRL_2;
    ctrl_2 &= ~(0x3u << 12);
    usart->CTRL_2 = ctrl_2;

    // Setup 8,N,1
    //
    std::uint32_t ctrl_1 = usart->CTRL_1;
    ctrl_1 |= ((0x1u << 2) | (0x1u << 3));
    usart->CTRL_1 = ctrl_1;

    // 115.2kb based on 16MHz clk and 16x oversampling
    //
    usart->BAUD_RATE = 0x8B;

    // Configure the Tx / Rx pins for the device
    //
    enable_usart_IO();

    enable();

    // --------------------------------------------------------
    // TO DO:
    // Configure the UART for interrupt operation here
    //
    // --------------------------------------------------------
  }

  void USART::enable()
  {
    std::uint32_t ctrl_1 = usart->CTRL_1;
    ctrl_1 |= (0x1u << 13);
    usart->CTRL_1 = ctrl_1;
  }


  void USART::disable()
  {
    std::uint32_t ctrl_1 = usart->CTRL_1;
    ctrl_1 &= ~(0x1u << 13);
    usart->CTRL_1 = ctrl_1;
  }


  void USART::enable_rx_interrupt()
  {
    std::uint32_t ctrl_1 = usart->CTRL_1;
    ctrl_1 |= (0x1u << 5);
    usart->CTRL_1 = ctrl_1;
  }


  void USART::enable_tx_interrupt()
  {
    std::uint32_t ctrl_1 = usart->CTRL_1;
    ctrl_1 |= (0x1u << 7);
    usart->CTRL_1 = ctrl_1;
  }


  void USART::disable_rx_interrupt()
  {
    std::uint32_t ctrl_1 = usart->CTRL_1;
    ctrl_1 &= ~(0x1u << 5);
    usart->CTRL_1 = ctrl_1;
  }


  void USART::disable_tx_interrupt()
  {
    std::uint32_t ctrl_1 = usart->CTRL_1;
    ctrl_1 &= ~(0x1u << 7);
    usart->CTRL_1 = ctrl_1;
  }

  USART::~USART()
  {
    disable();
    STM32F407::disable(STM32F407::USART_3);
  }


  char USART::read()
  {
    return static_cast<char>(usart->DATA);
  }


  void USART::write(char chr)
  {
    usart->DATA = static_cast<uint32_t>(chr);
  }


  void USART::send(char c)
  {
    while ((usart->STATUS & (0x1u << 7)) == 0)
    {
      ; // Wait...
    }
    write(c);
  }


  bool USART::try_get(char& chr)
  {
    // --------------------------------------------------------
    // TO DO:
    // Configure the UART for interrupt operation here
    //
    // --------------------------------------------------------

    if((usart->STATUS & (0x1u << 5)) != 0)
    {
      chr = read();
      return true;
    }
    else
    {
      return false;
    }
  }


  char USART::get_char()
  {
    bool success;
    char chr;
    do
    {
      success = try_get(chr);
    } while(!success);
    return chr;
  }


  void USART::send(const char* str)
  {
    while (*str != '\0')
    {
      this->send(*str++);
    }
  }

  // -----------------------------------------------------------------------------
  // Each UART requires two GPIO pins to be reconfigured
  // to act as the Tx and Rx pins.  These pins are on a
  // different port, and indeed different pins, for each
  // UART.
  // Tx pin is GPIO_B Pin 10
  // Rx pin is GPIO_B Pin 11

  // struct GPIO_registers
  // {
  //   std::uint32_t mode;
  //   std::uint32_t type;
  //   std::uint32_t speed;
  //   std::uint32_t pull_up_down;
  //   std::uint32_t input_data;
  //   std::uint32_t output_data;
  //   std::uint32_t bit_set_reset;
  //   std::uint32_t lock;
  //   std::uint32_t alt_fn_low;
  //   std::uint32_t alt_fn_high;
  // };

  // static constexpr uint32_t GPIO_B_addr
  // {
  //   STM32F407::AHB1_base + (0x400 * STM32F407::GPIO_B)
  // };

  // static volatile GPIO_registers* const gpio_B
  // {
  //   reinterpret_cast<GPIO_registers*>(GPIO_B_addr)
  // };


  void USART::enable_usart_IO()
  {
    USART_config::usart_enable_IO();
    // STM32F407::enable(STM32F407::GPIO_B);  // Enable GPIO port

    // std::uint32_t alt_fn_high = gpio_B->alt_fn_high;
    // alt_fn_high  |= (0x07u << (4 * (10 % 8)));   // ALT_FN7 - USART 3
    // alt_fn_high  |= (0x07u << (4 * (11 % 8)));   //
    // gpio_B->alt_fn_high = alt_fn_high;

    // std::uint32_t mode = gpio_B->mode;
    // mode |= (0x02u << (10 * 2));         // ALT FUNCTION
    // mode |= (0x02u << (11 * 2));         //
    // gpio_B->mode = mode;

    // std::uint32_t speed = gpio_B->speed;
    // speed |= (0x03u << (10 * 2));         // HIGH SPEED
    // speed |= (0x03u << (11 * 2));         //
    // gpio_B->speed = speed;

    // std::uint32_t pull_up_down = gpio_B->pull_up_down;
    // pull_up_down |= (0x01u << (10 * 2));         // PULL UP
    // pull_up_down |= (0x01u << (11 * 2));         //
    // gpio_B->pull_up_down = pull_up_down;
  }

} // namespace STM32F407
