// -----------------------------------------------------------------------------
// memory_map.h
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
#ifndef MEMORY_MAP_H_
#define MEMORY_MAP_H_

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
