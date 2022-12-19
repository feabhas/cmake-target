cmake_minimum_required(VERSION 3.21)
project(target-cortexm LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(USE_RTOS "Enable RTOS support" OFF)

set(CLANG_TIDY_OPTIONS -checks=-*,readability-* CACHE STRING "clang-tidy options" )

# uncomment this line to include clang_tidy as part of 
# the compilation phase
# set(CMAKE_CXX_CLANG_TIDY clang-tidy ${CLANG_TIDY_OPTIONS})

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

add_compile_options(
    -Wall
    -Wextra
    -Wconversion
    -Wsign-conversion
    # -Werror
    $<$<CONFIG:DEBUG>:-g3>
    $<$<CONFIG:DEBUG>:-Og>
    $<$<CONFIG:RELEASE>:-O3>
    $<IF:$<AND:$<BOOL:${USE_RTOS}>,$<COMPILE_LANGUAGE:CXX>>,-fexceptions,-fno-exceptions>
)

add_compile_definitions(
    $<$<CONFIG:DEBUG>:DEBUG>
    $<$<CONFIG:DEBUG>:TRACE_ENABLED>
    $<$<CONFIG:RELEASE>:NDEBUG>            # GCC/G++ disables asserts
    $<$<BOOL:${USE_RTOS}>:RTOS>
)

# Standard configuration - change with care

FILE (GLOB USER_SRC ${CMAKE_SOURCE_DIR}/src/*.c ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(Application ${USER_SRC})

set(LINKER_SCRIPTS 
    ${CMAKE_SOURCE_DIR}/ldscripts/mem.ld 
    ${CMAKE_SOURCE_DIR}/ldscripts/sections.ld
)

set_target_properties(Application PROPERTIES
    SUFFIX .elf
    LINK_DEPENDS "${LINKER_SCRIPTS}"
)

target_link_options(Application PRIVATE
  -T${CMAKE_SOURCE_DIR}/ldscripts/mem.ld
  -T${CMAKE_SOURCE_DIR}/ldscripts/sections.ld
  LINKER:--print-memory-usage
  LINKER:--sort-section=name
  LINKER:--cref
  LINKER:-Map,${CMAKE_CURRENT_BINARY_DIR}/Application.map
)

target_include_directories(Application PRIVATE
  src
  include
)

if (IS_DIRECTORY ${CMAKE_SOURCE_DIR}/system)
  add_subdirectory(system)
  target_link_libraries(Application PRIVATE system)
endif()

if (USE_RTOS AND IS_DIRECTORY ${CMAKE_SOURCE_DIR}/middleware)
  add_subdirectory(middleware)
  target_link_libraries(Application PRIVATE middleware)
endif()

if (EXISTS ${CMAKE_SOURCE_DIR}/src/main.c AND IS_DIRECTORY ${CMAKE_SOURCE_DIR}/drivers-c)
  add_subdirectory(drivers-c)
  target_link_libraries(Application PRIVATE drivers-c)
endif()

if (EXISTS ${CMAKE_SOURCE_DIR}/src/main.cpp  AND IS_DIRECTORY ${CMAKE_SOURCE_DIR}/drivers-cpp)
  add_subdirectory(drivers-cpp)
  target_link_libraries(Application PRIVATE drivers-cpp)
endif()

# Post build processing

if (EXISTS "${TOOLCHAIN_SIZE}")
  add_custom_command(
    TARGET Application
    POST_BUILD
    COMMAND ${TOOLCHAIN_SIZE} --format=berkeley $<TARGET_FILE:Application> 
            >${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_NAME:Application>.bsz
  )

  add_custom_command(
    TARGET Application
    POST_BUILD
    COMMAND ${TOOLCHAIN_SIZE} --format=sysv -x $<TARGET_FILE:Application> 
            >${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_NAME:Application>.ssz
  )
else()
  message(STATUS "'size' not found: cannot generate .[bs]sz files")
endif()

if (EXISTS ${CMAKE_OBJCOPY})
  add_custom_command(
    TARGET Application
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:Application> 
            ${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_NAME:Application>.hex
  )
else()
  message(STATUS "'objcopy' not found: cannot generate .hex file")
endif()

# static analysis targets

if (CLANG_TIDY)
    add_custom_target(
        clang-tidy-check clang-tidy -p ${CMAKE_BINARY_DIR}/compile_commands.json ${CLANG_TIDY_OPTIONS} ${USER_SRC}
        DEPENDS ${USER_SRC})
endif()

# optional testing

if (IS_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)
  enable_testing()
  add_subdirectory(tests)
endif()

