cmake_minimum_required(VERSION 3.16)
project(target-drivers-c LANGUAGES C)

add_library(drivers-c STATIC 
    timer.c         
    usart_utils.c
    usart_buffer.c  
)

target_include_directories(drivers-c INTERFACE
    .
)

target_link_libraries(drivers-c PRIVATE system)

if (USE_RTOS)
    target_link_libraries(drivers-c PRIVATE middleware)
endif()
