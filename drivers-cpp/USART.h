// -----------------------------------------------------------------------------
// USART.h
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
#ifndef USART_H_
#define USART_H_

namespace STM32F407
{
  class USART
  {
  public:
    USART();
    virtual ~USART();

    void send(char c);
    void send(const char* str);
    char get_char();
    virtual bool try_get(char& chr);

  protected:
    void enable();
    void disable();
    void enable_rx_interrupt();
    void enable_tx_interrupt();
    void disable_rx_interrupt();
    void disable_tx_interrupt();
    char read();
    void write(char chr);

  private:
    void enable_usart_IO();

    // UART configuration registers
    //
    volatile struct USART_registers* usart;
  };

} // namespace STM32F407

#endif /* USART_H_ */
