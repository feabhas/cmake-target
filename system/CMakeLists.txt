cmake_minimum_required(VERSION 3.16)
project(target-system LANGUAGES C CXX)

add_library(system OBJECT 
    ${PROJECT_SOURCE_DIR}/src/newlib/_syscalls.c
    ${PROJECT_SOURCE_DIR}/src/newlib/_startup.c
    ${PROJECT_SOURCE_DIR}/src/newlib/_sbrk.c
    ${PROJECT_SOURCE_DIR}/src/newlib/assert.c
    ${PROJECT_SOURCE_DIR}/src/newlib/__dso_handle.c
    ${PROJECT_SOURCE_DIR}/src/newlib/_exit.c
    ${PROJECT_SOURCE_DIR}/src/newlib/_write.c
    ${PROJECT_SOURCE_DIR}/src/cortexm/exception_handlers.c
    ${PROJECT_SOURCE_DIR}/src/cortexm/_reset_hardware.c
    ${PROJECT_SOURCE_DIR}/src/cortexm/_initialize_hardware.c
    ${PROJECT_SOURCE_DIR}/src/diag/Trace.c
    ${PROJECT_SOURCE_DIR}/src/diag/trace_impl.c
    ${PROJECT_SOURCE_DIR}/src/cmsis/vectors_stm32f4xx.c
    ${PROJECT_SOURCE_DIR}/src/cmsis/system_stm32f4xx.c
)

target_include_directories(system INTERFACE
    ${PROJECT_SOURCE_DIR}/include/cmsis
)

target_include_directories(system PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/cmsis
    ${PROJECT_SOURCE_DIR}/include/cortexm
    ${PROJECT_SOURCE_DIR}/include/diag
)
