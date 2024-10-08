#!/bin/bash

DISCLAIMER='// main program
// See project README.md for disclaimer and additional information.
// Feabhas Ltd
'
set -o nounset 
set -o errexit

CMAKE='cmake'
if ! $CMAKE --version >/dev/null 2>&1; then
    echo "Cannot find cmake command: $CMAKE" >&2
    exit 1
fi

GENERATOR='Ninja'
BUILD_OPTIONS=
BUILD_VERBOSE='-v'

# Ninja build does not work with C++20 Modules
if [[ -n $(type -p ninja) && ! -f src/Modules.txt ]]; then
  echo "Build generator: ninja $(ninja --version)"
elif [[ -n $(type -p make) ]]; then
  echo "Build generator: $(make --version | head -1)"
  GENERATOR='Unix Makefiles'
  BUILD_OPTIONS='--no-print-directory'
  BUILD_VERBOSE='VERBOSE=1'
else
  echo "Cannot find 'ninja' or GNU 'make' command:" >&2
  exit 1
fi

function usage {
  cat <<EOT
Usage: ${0#.*/} [options...]
  Wrapper around cmake build system.
  Generate options:
    clean      -- remove object files
    reset      -- regenerate make files and do a clean build
    debug      -- build debug version (default)
    release    -- build release version
    --rtos     -- include RTOS middleware if not found automatically
    --exc      -- enable exceptions also --exceptions
    --Cnn      -- set C langauge version to nn, also -c
    --C++nn    -- set C langauge version to nn, also -cpp -CPP
    -Dvar=val  -- define a CMake variable which must have a value  
  Build options: 
    --verbose  -- show compiler commands also -v
    clean      -- remove object files and build
    test       -- run cmake with test target after a build
    clang-tidy -- run clang-tidy after a build 
  Other options:
    --c        -- generate main.c if it doesn't exist
    --cpp      -- generate main.cpp if it doesn't exist
    --help     -- this help information also -h -?
    -Werror    -- adds -Werror to build

  
  Output written to build/debug (or build/release), executables:
      host:       build/debug/Application
      arm target: build/debug/Application.elf
  Generates compile_commands.json used by tools like clang-tidy.
  Script will generate a missing main.c/main.cpp based on the
  hostname: those staring with c- or ac- are C otherwise C++.
EOT
  exit 1
}

function main_template {
    LANG="${1:-}"
    if [[ -z $LANG ]]; then
      LANG=cpp
      if [[ $HOSTNAME == *c-* ]]; then
          LANG=c
      elif test -d *c-*_exercises; then
          LANG=c
      else
        for dir in solutions/01* exercises/solutions/01*; do
          if [[ -f $dir/main.c || -f $dir/src/main.c ]]; then
            LANG=c
            break
          fi
        done
      fi
    fi
    if [[ $LANG = c ]]; then
        echo "Generating src/main.c"
        cat >src/main.c <<EOT
${DISCLAIMER}
#include <stdio.h>
#include <stdbool.h>
#include <timer.h>

int main(void)
{
    for (unsigned i=0; i< 5; ++i) {
        puts("tick...");
        sleep(1000);
    };
    return 0;
}
EOT
    else
        echo "Generating src/main.cpp"
        cat >src/main.cpp <<EOT
${DISCLAIMER}
#include <iostream>
#include "Timer.h"

int main()
{
    for(unsigned i=0; i< 5; ++i) {
        std::cout << "tick...\n";
        sleep(1000);
    };
}
EOT
    fi
}

BUILD=build
CONFIG=debug
VERBOSE=
TEST=
CLANG_TIDY=
RESET=
CLEAN=
CMAKE_OPTS=
LANG=
RTOS=
EXC=

for arg; do
  case "$arg" in
    --help|-h|-\?) usage    ;;
    --verbose|-v)  VERBOSE="$BUILD_VERBOSE" ;;
    debug)         CONFIG=debug ;;
    release)       CONFIG=release ;;
    test)          TEST=1   ;;
    clang-tidy)    CLANG_TIDY=1 ;;
    clean)         CLEAN=1  ;;
    reset)         RESET=1  ;;
    --[cC])        LANG=c   ;;
    --cpp|--CPP|--[cC]++)   
                   LANG=cpp ;;
    --[cC][0-9][0-9]) 
                   CMAKE_OPTS="$CMAKE_OPTS -DCMAKE_C_STANDARD=${arg#--[cC]}" ;;
    --[cC]++[0-9][0-9]|--CPP[0-9][0-9]|--cpp[0-9][0-9]) 
                   CMAKE_OPTS="$CMAKE_OPTS -DCMAKE_CXX_STANDARD=${arg#--[cC]??}" ;;
    --exc|--exceptions)  
                   EXC='-DEXCEPTIONS=ON' ;;
    --rtos)        RTOS='-DRTOS=ON' ;;
    -Werror)       CMAKE_OPTS="$CMAKE_OPTS -DCMAKE_CXX_FLAGS=-Werror -DCMAKE_C_FLAGS=-Werror"  ;;
    -D*)           CMAKE_OPTS="$CMAKE_OPTS $arg" ;;
    *)
      echo "Unknown option $arg" >&2
      usage
      ;;
  esac
done

# generate main.c/main.cpp if required

[[ ! -d src ]] && mkdir src
[[ ! -d include ]] && mkdir include

if [[ ! -f src/main.c && ! -f src/main.cpp ]]; then
  main_template $LANG
fi

# check for rtos required

if grep -iq 'feabhOS[a-z_0-9]*\.h' src/*.c src/*.cpp src/*.h include/*.h 2>/dev/null || \
    grep -q 'using[^;]*FeabhOS' src/*.cpp src/*.h include/*.h 2>/dev/null; then
    RTOS='-DRTOS=ON'
fi

# check if rtos not used before

if [[ -n $RTOS ]]; then
    if ! grep -q 'RTOS:BOOL=ON' $BUILD/$CONFIG/CMakeCache.txt 2>/dev/null; then
        RESET=1
    fi
    [[ -f src/main.cpp ]] && EXC='-DEXCEPTIONS=ON'
fi

# check for exceptions not used

if [[ -n $EXC ]]; then
    if ! grep -q 'EXCEPTIONS:UNINITIALIZED=ON' $BUILD/$CONFIG/CMakeCache.txt 2>/dev/null; then
        RESET=1
    fi
fi

# force clean generate

FSTAMP='.files.md5'
old=
if [[ -f "$FSTAMP" ]]; then
    old=$(cat $FSTAMP)
fi
find include src -name '*.[ch]' -o -name '*.cpp' | md5sum >$FSTAMP
new=$(cat $FSTAMP)
[[ "$old" != "$new" ]] && RESET=1

if [[ -n $RESET && -d $BUILD/$CONFIG ]]; then
  rm -rf $BUILD/$CONFIG
elif [[ ! -d $BUILD/$CONFIG ]]; then
  RESET=1
fi

# run cmake

if [[ -n $RESET ]]; then
    $CMAKE --preset ${CONFIG} -G "$GENERATOR"  $CMAKE_OPTS $RTOS $EXC
fi

if [[ -n $CLEAN ]]; then
  $CMAKE --build --preset ${CONFIG} --target clean
fi

if $CMAKE --build --preset ${CONFIG} -- $BUILD_OPTIONS $VERBOSE
then
  if [[ -n $CLANG_TIDY ]]; then
    $CMAKE --build --preset clang-tidy
  fi
  if [[ -n $TEST ]]; then
    $CMAKE --build --preset test
  fi
else
  exit $?
fi

