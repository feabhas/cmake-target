#!/bin/bash
DISCLAIMER='
// -----------------------------------------------------------------------------
// main program
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
'
set -o nounset 
set -o errexit

CMAKE='cmake'
if ! $CMAKE --version >/dev/null 2>&1; then
    echo "Cannot find cmake command: $CMAKE" >&2
    exit 1
fi

BUILD=build
TYPE=DEBUG
BUILD_DIR=$BUILD/debug
VERBOSE=
FILE_LIST=filelist.txt
TEST=
CLANG_TIDY=

function usage {
  cat <<EOT
Usage: ${0#.*/} [options...]
  Wrapper around cmake build system.
  Options:
    clean      -- remove object files
    reset      -- regenerate make files and do a clean build
    debug      -- build debug version (default)
    release    -- build release version
    test       -- run cmake with test target after a build
    clang-tidy -- run clang-tidy after a build
    --c        -- generate main.c if it doesn't exist
    --cpp      -- generate main.cpp if it doesn't exist
    --verbose  -- add verbose option also -v
    --help     -- this help information also -h -?
    -Werror    -- adds -Werror to build
    -Dvar=val  -- define a CMake variable which must have a value
  
  Output written to build/debug (or build/release), executables:
      host:       build/debug/Application
  Generates compile_commands.json used by tools like clang-tidy.
  Set clang_tidy options using -DCLANG_TIDY_OPTIONS="options"
  Script will generate a missing main.c/main.cpp based on the
  hostname: those staring with c- or ac- are C otherwise C++.
EOT
  exit 1
}

function main_template {
    LANG="${1:-}"
    if [[ -z $LANG ]]; then
        if [[ $HOSTNAME == *c-* ]]; then
            LANG=c
        else
            LANG=cpp
        fi
    fi
    if [[ $LANG = c ]]; then
        echo "Generating src/main.c"
        cat >src/main.c <<EOT
${DISCLAIMER}
#include <stdio.h>

int main(void)
{
    printf("Hello world!\n");
    return 0;
}
EOT
    else
        echo "Generating src/main.cpp"
        cat >src/main.cpp <<EOT
${DISCLAIMER}
#include <iostream>

int main()
{
    std::cout << "Hello world!" << std::endl;
    return 0;
}
EOT
    fi
}

RESET=
CLEAN=
CMAKE_OPTS=
LANG=

for arg; do
  case "$arg" in
    --help|-h|-\?) usage    ;;
    --verbose|-v)  VERBOSE='VERBOSE=1'  ;;
    debug)         TYPE=DEBUG;   BUILD_DIR=$BUILD/debug ;;
    release)       TYPE=RELEASE; BUILD_DIR=$BUILD/release  ;;
    test)          TEST=1   ;;
    clang-tidy)    CLANG_TIDY=1 ;;
    clean)         CLEAN=1  ;;
    reset)         RESET=1  ;;
    --c)           LANG=c   ;;
    --cpp)         LANG=cpp ;;
    -Werror)       CMAKE_OPTS="$CMAKE_OPTS -DCMAKE_CXX_FLAGS=-Werror -DCMAKE_C_FLAGS=-Werror"  ;;
    -D*)           CMAKE_OPTS="$CMAKE_OPTS $arg" ;;
    *)
      echo "Unknown option $arg" >&2
      usage
      ;;
  esac
done

# force clean generate

if [[ -n $RESET && -d $BUILD_DIR ]]; then
  rm -rf $BUILD_DIR
elif [[ ! -d $BUILD_DIR ]]; then
  RESET=1
fi

# generate main.c/main.cpp if required

[[ ! -d src ]] && mkdir src
[[ ! -d include ]] && mkdir include

if [[ ! -f src/main.c && ! -f src/main.cpp ]]; then
  main_template $LANG
fi


# run cmake

$CMAKE -S . -B $BUILD_DIR --warn-uninitialized -DCMAKE_BUILD_TYPE=$TYPE $CMAKE_OPTS

if [[ -n $CLEAN ]]; then
  $CMAKE --build $BUILD_DIR --target clean
fi

if $CMAKE --build $BUILD_DIR --config Debug -- --no-print-directory $VERBOSE
then
  if [[ -n $CLANG_TIDY ]]; then
    $CMAKE --build $BUILD_DIR --target clang-tidy-check
  fi

  if [[ -n $TEST ]]; then
    $CMAKE --build $BUILD_DIR -- test -- ARGS="--output-on-failure"
  fi
else
  exit $?
fi


