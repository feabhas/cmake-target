cmake_minimum_required(VERSION 3.16)
project(target-drivers-cpp LANGUAGES CXX)

add_library(drivers-cpp OBJECT 
    Event.cpp 
    Peripherals.cpp  
    Timer.cpp
    USART.cpp
    USART_utils.cpp
)

target_include_directories(drivers-cpp PUBLIC
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(drivers-cpp PRIVATE system)

if (USE_RTOS)
    target_link_libraries(drivers-cpp PRIVATE middleware)
endif()
