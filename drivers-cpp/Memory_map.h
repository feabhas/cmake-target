// Memory_map.h
// See project README.md for disclaimer and additional information.
// Feabhas Ltd

#pragma once
#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

#include <cstdint>
#include "Peripherals.h"

namespace STM32F407
{
  // Base address for devices on the STM32F10x
  //
  constexpr uint32_t Flash_base      { 0x08000000 };            // FLASH base address
  constexpr uint32_t SRAM_base       { 0x20000000 };            // SRAM base address in the alias region
  constexpr uint32_t Peripheral_base { 0x40000000 };            // Peripheral base address in the alias region

  // Peripheral memory map
  //
  constexpr uint32_t APB1_base   { Peripheral_base + 0x00000 }; // Advanced Peripheral Bus 1
  constexpr uint32_t APB2_base   { Peripheral_base + 0x10000 }; // Advanced Peripheral Bus 2
  constexpr uint32_t AHB1_base   { Peripheral_base + 0x20000 }; // Advanced High-performance Bus 1

  constexpr uint32_t device_base_address(STM32F407::AHB1_Device device) {
    return STM32F407::AHB1_base + (0x400 * device);
  }

  constexpr uint32_t device_base_address(STM32F407::APB1_Device device) {
    return STM32F407::APB1_base + (0x400 * device);
  }

  constexpr uint32_t device_base_address(STM32F407::APB2_Device device) {
    return STM32F407::APB2_base + (0x400 * device);
  }

} // namespace STM32F407

#endif // MEMORY_MAP_H_
