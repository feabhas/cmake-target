#!/bin/bash

# The Build all script is intended for checking projects and solutions
# uploaded to a GIT repo for continuous integration checks.

# It looks for solutions to exercises in a "solutions" folder
# in a set of standard locations:

SOL_PATH=".. ../.. $HOME $HOME/course_material ."

# Solutions must be in folder names starting or ending with a digit and optional letter.
# If a solution folder contains a MakeLists.txt file a CMake build is run
# in the solution folder. If the solution contains an SConstruct file a
# scons build is run. Otherwise the build-one.sh command is run to
# build the solution.

set -o nounset 
set -o errexit

# configurable variables

CMAKE_BUILD="./build.sh reset"
SCONS_BUILD="scons -c; scons"

# command line options for RTOS

SCONS_RTOS="$SCONS_BUILD --rtos"
BUILD=
BUILD_RTOS=

SRC=src
SRCBAK=src.bak

function err_exit {
  [[ -n ${1:-} ]] && echo "$1" >&2
  echo "Build script for exercise solutions" >&2
  echo "Usage: $0 [-q|--quiet] [--scons|--cmake] [-A|--all]  [ NN ]" >&2
  echo "  where NN is the chapter number, or" >&2
  echo "  -A --all        to build all solutions" >&2
  echo "  -v --verbose    show diagnostic trace messages" >&2
  echo "  -q --quiet      to suppress output messages, a build failure is always reported" >&2
  echo "  --scons --cmake to override default build system" >&2
  exit 1
}

function trace {
  [[ -z $QUIET ]] && echo $1
  : # required due to bug in handling &&
}

function get_solutions {
  SOLDIR=
  for dir in $SOL_PATH ; do
    [[ -d $dir/solutions ]] && SOLDIR="$dir/solutions" && break
  done
  [[ -z $SOLDIR ]] && err_exit "Cannot find solutions folder"
  : # required due to bug in handling &&
}

function get_all_exercises {
  EXERCISES=
  EXDIR=$(ls "$1" | egrep '^[0-9]|[0-9][A-Z]?$') 
  for ex in $EXDIR; do
    n=$(echo $ex | sed -rn 's/^(0?[1-9][0-9]?[^0-9])|(0?[1-9][0-9]?[A-Z]?)$/\1\2/p')
    EXERCISES="$EXERCISES $n"
  done
}

VERBOSE=
EXERCISES=
SOLDIR=
REDIR=/dev/stdout
QUIET=
ALL=

while getopts ":vqhA-:" opt; do
  [[ $opt == - ]] && opt="--$OPTARG"
  [[ $opt == \? && $OPTARG == \? ]] && opt="--help"
  [[ $opt == ? ]] && opt="-$opt"
  case $opt in
    -A|--all) 
      ALL=1
      get_solutions 
      get_all_exercises $SOLDIR
      ;;
    -v|--verbose) 
      VERBOSE=y
      ;;
    -q|--quiet) 
      REDIR=/dev/null
      QUIET=y
      ;;
    --scons) 
      [[ -n $BUILD ]] && err_exit "Cannot specify more than one build system"
      [[ ! -f SConstruct ]] && err_exit "Missing SConstruct file required by scons"
      BUILD="$SCONS_BUILD"
      BUILD_RTOS="$SCONS_RTOS"
      ;;
    --cmake) 
      [[ -n $BUILD ]] && err_exit "Cannot specify more than one build system"
      [[ ! -f CMakeLists.txt ]] && err_exit "Missing CMakeLists.txt file required by cmake"
      BUILD="$CMAKE_BUILD"
      BUILD_RTOS="$SCMAKE_BUILD"
      ;;
    --help) 
      err_exit 
      ;;
    *) 
      err_exit "Unknown option '$OPTARG'"
      ;;
  esac
done
shift $(( OPTIND - 1 ))

[[ -z $SOLDIR ]] && get_solutions 
[[ -z $SOLDIR ]] && err_exit "Cannot find solutions folder"


[[ -z $EXERCISES && $# == 0 ]] && err_exit "No solution number specified"
[[ -z $EXERCISES && $# != 1 ]] && err_exit "Cannot specify multiple solution numbers"
[[ -n $EXERCISES && $# != 0 ]] && err_exit "Cannot specify -A (-all) and a solution number"

if [[ -z $EXERCISES ]]; then
  case $1 in
    [1-9]|[1-9][A-Z]) 
      EXERCISES="0$1"
      ;;
    [0-9][0-9]|[0-9][0-9][A-Z]) 
      EXERCISES="$1"
      ;;
    *)
      if [[ ! -d "$SOLDIR/$1" ]]; then
        err_exit "Invalid solution number: $1"
      fi
      EXERCISES="$1"
      ;;
  esac
fi

[[ -f CMakeLists.txt && ! -d $SRC ]] && mkdir $SRC
[[ ! -d $SRC ]] && err_exit "please run this script inside the root of your workspace"

if [[ -d $SRCBAK ]]; then
  rm -rf "$SRCBAK"/* 2>/dev/null
else
  trace "Making source backup folder $SRCBAK"
  mkdir "$SRCBAK"
fi

for EX in $EXERCISES; do
  trace "Moving source files to $SRCBAK"
  mv "$SRC"/* "$SRCBAK" 2>/dev/null || true
  
  if [[ -d "$SOLDIR/$EX" ]]; then
    EXDIR=$EX
  else
    EXDIR=$(ls "$SOLDIR" | egrep "^0?${EX}[^0-9]|0?${EX}$") || true
    [[ -z $EXDIR ]] && err_exit "No solution provided for exercise $EX"
    (cd $SOLDIR; ls -d $EXDIR); 
    N=$(cd $SOLDIR; ls -d $EXDIR | wc -l)
    (( N > 1 )) && err_exit "Configuration error - multiple solutions for exercise $EX"
  fi
  
  if [[ -d "$SOLDIR/$EXDIR/src" ]]; then
    EXDIR="$EXDIR/src"
  fi

  trace "Copying solution files from $SOLDIR/$EXDIR"
  cp -f "$SOLDIR/$EXDIR"/* $SRC 2>/dev/null || true

  RTOS=
  if grep -iq 'feabhOS[a-z_0-9]*\.h' $SRC/*.c $SRC/*.cpp 2>/dev/null; then
    RTOS=1
  fi
  
  if [[ -n $VERBOSE ]]; then
    echo "checking $EX" >&2
  fi
  
  set +o errexit

  if [[ -n $BUILD ]]; then
    [[ -n $RTOS ]] && BUILD="$BUILD_RTOS"
    trace "Building solutions using $BUILD"
    (eval $(echo "$BUILD")) >$REDIR 2>&1
  elif [[ -f CMakeLists.txt ]]; then
    trace "Running cmake build: $CMAKE_BUILD"
    (eval $(echo "$CMAKE_BUILD")) >$REDIR 2>&1
  elif [[ -f SConstruct ]]; then
    [[ -n $RTOS ]] && SCONS_BUILD="$SCONS_RTOS"
    trace "Running scons build: $SCONS_BUILD"
    (eval $(echo "$SCONS_BUILD")) >$REDIR 2>&1
  else
    err_exit "Cannot file suitable build configuration file"
  fi
  status=$?
  if [[ $status != 0 ]]; then
    echo "Build failed for solution $EX"
    exit $status
  fi
  set -o errexit
done

