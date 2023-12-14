// Peripherals.cpp
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#include "Peripherals.h"
#include "Memory_map.h"

#include <cstdint>
using std::uint32_t;

namespace
{
    inline void enable_device(volatile uint32_t *const  rcc, unsigned device)
    {
      uint32_t port = *rcc;
      port |= (0x1u << device);
      *rcc = port;
    }

    inline void disable_device(volatile uint32_t *const  rcc, unsigned device)
    {
      uint32_t port = *rcc;
      port &= ~(0x1u << device);
      *rcc = port;
    }
}

namespace STM32F407
{
    // Use static consts to give the compiler
    // the best opportunity to optimise
    //
    constexpr uint32_t RCC_base { AHB1_base + 0x3800 };

    static volatile uint32_t* RCC_AHB1_enable  { reinterpret_cast<uint32_t*>(RCC_base + 0x30) };
    static volatile uint32_t* RCC_APB1_enable  { reinterpret_cast<uint32_t*>(RCC_base + 0x40) };
    static volatile uint32_t* RCC_APB2_enable  { reinterpret_cast<uint32_t*>(RCC_base + 0x44) };

    void enable(AHB1_Device device)
    {
      enable_device(RCC_AHB1_enable, unsigned(device));
    }

    void enable(APB1_Device device)
    {
      enable_device(RCC_APB1_enable, unsigned(device));
    }

    void enable(APB2_Device device)
    {
      enable_device(RCC_APB2_enable, unsigned(device));
    }

    void disable(AHB1_Device device)
    {
      disable_device(RCC_AHB1_enable, unsigned(device));
    }

    void disable(APB1_Device device)
    {
      disable_device(RCC_APB1_enable, unsigned(device));
    }

    void disable(APB2_Device device)
    {
      disable_device(RCC_APB2_enable, unsigned(device));
    }

} // namespace STM32F407
