// USART.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef USART_H
#define USART_H

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
