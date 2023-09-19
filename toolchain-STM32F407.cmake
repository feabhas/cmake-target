# set CMAKE_SYSTEM_NAME to define build as CMAKE_CROSSCOMPILING
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION Cortex-M4-STM32F407)
set(CMAKE_SYSTEM_PROCESSOR arm)

# settinge either of the compilers builds the absolute paths for the other tools:
#   ar, nm, objcopy, objdump, ranlib, readelf -- but not as, ld, size
# if the compiler cannot be found the try_compile() function will fail the build
# set(CMAKE_C_COMPILER arm-none-eabi-gcc)
# set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

# we need to get compiler path itself, for the toolchain
# but still rely on try_compile to check we have a valid compiler

# TOOLCHAIN EXTENSION
IF(WIN32)
    SET(TOOLCHAIN_EXT ".exe")
ELSE()
    SET(TOOLCHAIN_EXT "")
ENDIF()

# TARGET_TRIPLET
SET(TARGET_TRIPLET "arm-none-eabi-")

find_program(CROSS_GCC_PATH ${TARGET_TRIPLET}gcc${TOOLCHAIN_EXT})
MESSAGE(STATUS "Using compiler: " ${CROSS_GCC_PATH})
cmake_path(GET CROSS_GCC_PATH PARENT_PATH TOOLCHAIN)

# WARNING: Arm keep chaging directory name formats
#          Check the pattern works when updating the Arm toolchains
string(REGEX MATCH "toolchain-1[2-9]\." ARM12 ${CROSS_GCC_PATH})
if (ARM12)
    MESSAGE(STATUS "Using ARM Toolchain 12. or later")
endif()

set(TOOLCHAIN_PREFIX ${TOOLCHAIN}/${TARGET_TRIPLET})

set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc${TOOLCHAIN_EXT})
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++${TOOLCHAIN_EXT})
set(TOOLCHAIN_AS ${TOOLCHAIN_PREFIX}as${TOOLCHAIN_EXT} CACHE STRING "${TARGET_TRIPLET}as")
set(TOOLCHAIN_LD ${TOOLCHAIN_PREFIX}ld${TOOLCHAIN_EXT} CACHE STRING "${TARGET_TRIPLET}ld")
set(TOOLCHAIN_SIZE ${TOOLCHAIN_PREFIX}size${TOOLCHAIN_EXT} CACHE STRING "${TARGET_TRIPLET}size")
set(TOOLCHAIN_SIZE ${TOOLCHAIN_PREFIX}size${TOOLCHAIN_EXT} CACHE STRING "${TARGET_TRIPLET}size")

# --specs= is both a compiler and linker option
set(ARM_OPTIONS -mcpu=cortex-m4 -mfloat-abi=soft 
  $<IF:$<BOOL:${EXCEPTIONS}>,--specs=rdimon.specs,--specs=nano.specs>
)

add_compile_options(
  ${ARM_OPTIONS}
  -Wno-psabi
  -fmessage-length=0
  -funsigned-char
  -ffunction-sections
  -fdata-sections
  # -fno-exceptions
  # -fno-move-loop-invariants -fstack-usage
  -MMD
  -MP)

add_compile_definitions(
  STM32F407xx
  USE_FULL_ASSERT
  $<$<CONFIG:DEBUG>:TRACE>
  # OS_USE_TRACE_SEMIHOSTING_DEBUG  # semi-hosting options mutually exclusive
  $<$<CONFIG:DEBUG>:OS_USE_TRACE_SEMIHOSTING_STDOUT>
  $<$<CONFIG:DEBUG>:OS_USE_SEMIHOSTING>)

# use this to avoid running the linker during test compilation
# set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# use these options to verify the linker can create an ELF file
# when not doing a static link

add_link_options(
  ${ARM_OPTIONS}
  $<$<CONFIG:DEBUG>:--specs=rdimon.specs> # use the semihosted version of the syscalls
  $<$<CONFIG:RELEASE>:--specs=nosys.specs> 
  $<$<CONFIG:DEBUG>:-u_printf_float>
  $<$<CONFIG:DEBUG>:-u_scanf_float>
  -nostartfiles
  LINKER:--gc-sections
  LINKER:--build-id
  $<$<BOOL:${ARM12}>:LINKER:--no-warn-rwx-segments>
)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
