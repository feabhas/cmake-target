cmake_minimum_required(VERSION 3.16)
project(host-gcc LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CLANG_TIDY_OPTIONS -checks=-*,readability-* CACHE STRING "clang-tidy options" )

# uncomment this line to include clang_tidy as part of 
# the compilation phase
# set(CMAKE_CXX_CLANG_TIDY clang-tidy ${CLANG_TIDY_OPTIONS})

if(MSVC)
add_compile_options(
    /W4
    $<$<CONFIG:DEBUG>:/Od>
    $<$<CONFIG:RELEASE>:/O2>
)
else()
add_compile_options(
    -Wall
    -Wextra
    -Wconversion
    -Wsign-conversion
    # -Werror
    $<$<CONFIG:DEBUG>:-g3>
    $<$<CONFIG:DEBUG>:-Og>
    $<$<CONFIG:RELEASE>:-O3>
)
endif()

add_compile_definitions(
    $<$<CONFIG:DEBUG>:DEBUG>
    $<$<CONFIG:DEBUG>:_DEBUG>     # MSVC enables assert
    $<$<CONFIG:RELEASE>:NDEBUG>   # GCC/G++ disables asserts
)

# WARNING: wildcards are expanded at generate time, not build time
# regenerate the build if new source files are added

file (GLOB USER_SRC ${CMAKE_SOURCE_DIR}/src/*.c ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(Application ${USER_SRC})

target_include_directories(Application PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/include
)

set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
target_link_libraries(Application PRIVATE Threads::Threads)

# strip release exe 

if (CMAKE_BUILD_TYPE STREQUAL Release)
  if ("${CMAKE_STRIP}")
    add_custom_command(
      TARGET Application
      POST_BUILD
      COMMAND ${CMAKE_STRIP} $<TARGET_FILE:Application>
    )
  else()
    message(STATUS "'strip' not found: cannot strip executable binary")
  endif()
endif()

# static analysis targets

find_program(CLANG_TIDY clang-tidy)

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

