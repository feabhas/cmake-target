cmake_minimum_required(VERSION 3.16)
project(target-middleware LANGUAGES C CXX)

add_library(middleware STATIC
    cortex_m4_config/feabhas_freertos.c

    FreeRTOSv202012.00/FreeRTOS/Source/croutine.c
    FreeRTOSv202012.00/FreeRTOS/Source/event_groups.c
    FreeRTOSv202012.00/FreeRTOS/Source/list.c
    FreeRTOSv202012.00/FreeRTOS/Source/queue.c
    FreeRTOSv202012.00/FreeRTOS/Source/stream_buffer.c
    FreeRTOSv202012.00/FreeRTOS/Source/tasks.c
    FreeRTOSv202012.00/FreeRTOS/Source/timers.c

    FreeRTOSv202012.00/FreeRTOS/Source/portable/GCC/ARM_CM3/port.c
    FreeRTOSv202012.00/FreeRTOS/Source/portable/MemMang/heap_3.c

    feabhos/C/platform/FreeRTOS/src/feabhOS_memory.c
    feabhos/C/platform/FreeRTOS/src/feabhOS_mutex.c
    feabhos/C/platform/FreeRTOS/src/feabhOS_signal.c
    feabhos/C/platform/FreeRTOS/src/feabhOS_task.c
    feabhos/C/platform/FreeRTOS/src/feabhOS_error.c
    feabhos/C/platform/FreeRTOS/src/feabhOS_semaphore.c
    feabhos/C/platform/FreeRTOS/src/feabhOS_eventflags.c
    feabhos/C/platform/FreeRTOS/src/feabhOS_condition.c
    feabhos/C/platform/FreeRTOS/src/feabhOS_queue.c
    feabhos/C/platform/FreeRTOS/src/feabhOS_mailbox.c
    feabhos/C/platform/FreeRTOS/src/feabhOS_scheduler.c
    feabhos/C/platform/FreeRTOS/src/feabhOS_interrupts.c
    feabhos/C/platform/FreeRTOS/src/feabhOS_rwlock.c
    feabhos/C/platform/FreeRTOS/src/feabhOS_rendezvous.c
    feabhos/C/platform/FreeRTOS/src/feabhOS_allocator.c
)

set (MIDDLEWARE_INC
    feabhos/C/inc
    feabhos/C/platform/FreeRTOS/inc
    feabhos/C++14/inc
    cortex_m4_config
    FreeRTOSv202012.00/FreeRTOS/Source/include
    FreeRTOSv202012.00/FreeRTOS/Source/portable/GCC/ARM_CM3
)

target_include_directories(middleware INTERFACE
    ${MIDDLEWARE_INC}
)

target_include_directories(middleware PRIVATE
    ${MIDDLEWARE_INC}
)

target_link_libraries(middleware PRIVATE system)
