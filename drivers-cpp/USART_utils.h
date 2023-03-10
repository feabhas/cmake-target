// -----------------------------------------------------------------------------
// USART_utils.h
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
#ifndef USART_UTILS_H_
#define USART_UTILS_H_

#include "Memory_map.h"
#include <cstdint>

namespace STM32F407
{
  // ---------------------------------------------------------------------
  // Namespace for configuration properties.
  //
  namespace USART_config
  {
    // ---------------------------------------------------------------------
    // USART register definitions
    //
    // Status register.
    // Bits in this register indicate
    // the current operation of the USART
    //
    struct Status
    {
      std::uint32_t PE : 1;   // Parity error
      std::uint32_t FE : 1;   // Framing error
      std::uint32_t NF : 1;   // Noise detected
      std::uint32_t ORE : 1;  // Overrun error
      std::uint32_t IDLE : 1; // Idle line detected
      std::uint32_t RXNE : 1; // Rx buffer not empty
      std::uint32_t TC : 1;   // Transmission complete
      std::uint32_t TXE : 1;  // Tx data register empty
      std::uint32_t LBD : 1;  // Line break detected
      std::uint32_t CTS : 1;  // Clear To Send
      std::uint32_t : 0;      // Reserved
    };

    // Data register.
    // The Transmit and Receive buffers are at the
    // same address in memory (hence the union).
    //
    union Data
    {
      std::uint32_t tx; // Transmit buffer (bits 0 - 8)
      std::uint32_t rx; // Receive buffer  (bits 0 - 8)
    };

    // Divisor.  The baud rate is based on the peripheral clock speed.
    // This has to be scaled down.  This register holds the clock
    // divisor for the required baud rate.
    //
    struct Divisor
    {
      std::uint32_t fraction : 4;  // Baud rate clock divisor fractional part
      std::uint32_t mantissa : 12; // Baud rate clock divisor mantissa
    };

    // Control Register 1.
    // This register enables / disables core functions on the USART.
    //
    struct Control_1
    {
      std::uint32_t SBK : 1;    // Send break
      std::uint32_t RWU : 1;    // Receiver wake-up
      std::uint32_t RE : 1;     // Receiver enable
      std::uint32_t TE : 1;     // Transmitter enable
      std::uint32_t IDLEIE : 1; // Idle interrupt enable
      std::uint32_t RXEIE : 1;  // Rx interrupt enable
      std::uint32_t TCIE : 1;   // Tx complete interrupt enable
      std::uint32_t TXEIE : 1;  // Tx buffer empty interrupt enable
      std::uint32_t PEIE : 1;   // Parity error interrupt enable
      std::uint32_t PS : 1;     // Parity select
      std::uint32_t PCE : 1;    // Parity control enable
      std::uint32_t WAKE : 1;   // Wake-up method
      std::uint32_t M : 1;      // Word length
      std::uint32_t UE : 1;     // USART enable
      std::uint32_t : 1;        // reserved
      std::uint32_t OVER8 : 1;  // Oversampling mode
      std::uint32_t : 16;       // Reserved
    };

    // Control Register 2.
    // This register focuses on data transmission properties
    //
    struct Control_2
    {
      std::uint32_t ADD : 4;   // USART node address
      std::uint32_t : 1;       // Reserved
      std::uint32_t LBDL : 1;  // Line break detection
      std::uint32_t LBDIE : 1; // Line break detection interrupt enable
      std::uint32_t : 1;       // Reserved
      std::uint32_t LBCL : 1;  // Last bit clock pulse
      std::uint32_t CPHA : 1;  // Clock phase
      std::uint32_t CPOL : 1;  // Clock polarity
      std::uint32_t CLKEN : 1; // Clock enable
      std::uint32_t STOP : 2;  // Number of stop bits
      std::uint32_t LINEN : 1; // LIN mode enable
      std::uint32_t : 17;      // Reserved
    };

    // ---------------------------------------------------------------------
    // Transmission configuration properties.
    // These enums are defined so that they match the underlying hardware
    // values.  That is, these enum values can be written directly to the
    // appropriate registers
    //
    enum class Baud_rate : std::uint32_t {
      baud_2400   = 0x1A0B,
      baud_9600   = 0x0683,
      baud_57600  = 0x0116,
      baud_115200 = 0x008B
    };

    enum class Data_length { eight, nine };

    enum class Stop_bits { one, half, two, one_and_a_half };

    enum class Parity_check { disabled, enabled };

    enum class Parity { even, odd };

    // ---------------------------------------------------------------------
    // USART register address offsets
    //
    enum class Register : std::uint32_t {
      status    = 0x00,
      data      = 0x04,
      divisor   = 0x08,
      control1  = 0x0C,
      control2  = 0x10,
      control3  = 0x14,
      prescaler = 0x18
    };

    // ---------------------------------------------------------------------
    // Helper functions
    //
    // Enable Tx and Rx pins for USART 3
    //
    void usart_enable_IO();
    void usart_configure();
    void usart_enable_rx_interrupts();

  } // namespace USART_config

} // namespace STM32F407

#endif // USART_UTILS_H_
