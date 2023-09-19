#!/bin/bash
# This script looks for exercise templates
# in a set of standard locations:

TEMPLATE_PATH="./exercises . ../exercises .. $HOME/exercises $HOME"
SOURCES="src include"

# All folders in the templates folder must contain a src folder with
# starter files for a C/C++ exercise which are copied to the src folder,
# nested inc or include folder filess are copied to the include folder.
# Existing src and include files are copied to the src.back folder
# overwriting an existing back files.
# The template may be built using the CMake in the current folder.

set -o nounset 
set -o errexit

function usage 
{
  echo >&2
  echo $1 >&2
  echo >&2
  cat  >&2 <<EOT
Usage: ${0#.*/} [exercises]
  Create new sub-projects in this folder copying from each 
  project in the 'templates' sub-folder of the 'exercises'
  folder given on the command line (default: this folder).
    --build  builds each template using CMake, also -b
    --keep   keeps original project structure, also -k
             default is to copy to a 'project' subfolder
    --help   this help message, also -h -?
EOT
  exit 1
}

function get_template_dir {
  TEMPLATE_DIR=
  for dir in $TEMPLATE_PATH ; do
    echo "Looking in $dir"
    if [[ -d $dir/templates ]]; then
      TEMPLATE_DIR="$dir/templates"
      break
    fi
  done
  [[ -z $TEMPLATE_DIR ]] && usage "Cannot find template folder"
  : # required due to bug in handling &&
}


TEMPLATE_DIR=
ALL=
BUILD=
SAVE=project
for arg; do
  case "$arg" in
    --help|-h|-\?) usage ;;
    --build|-b) BUILD=1 ;;
    --keep|-k) SAVE= ;;
    *)
      if [[ -n "$TEMPLATE_DIR" ]]; then
          usage "Unknown argument $arg: template already set to $TEMPLATE_DIR"
      fi
      TEMPLATE_DIR="$arg"
      ;;
  esac
done 

[[ ! -e build.sh ]] && usage "No build.sh file found - cannot create template projects"

[[ -z "$TEMPLATE_DIR" ]] && get_template_dir
SOL=
DIRS=()

SOLUTIONS_DIR=$(dirname $TEMPLATE_DIR)/solutions
  
[[ -d build ]] && rm -rf build

SCRIPT="$(basename $0)"
FILES=
for file in *; do
  case "$file" in
    exercises|templates|solutions|LICENSE*|"$SCRIPT") continue;;
  esac
  # check if generated template
  [[ -f "$file/build.sh" ]] && continue
  FILES="$file $FILES"
done
for dir in .vscode .devcontainer; do
  [[ -d "$dir" ]] && FILES="$dir $FILES"
done

rm -rf src/* include/*
  
for t in "$TEMPLATE_DIR"/*; do
  [[ ! -d "$t/src" ]] && continue
  name=$(basename $t)
  name=${name,,}
  if [[ -d "$name" ]]; then
    echo "Subfolder '$name' already exists - skipping template $(basename $t)"
    continue 
  fi
  echo "Cloning project $name from template $t"
   
  for dir in $SOURCES; do
    if [[ -d "$t/$dir" ]]; then
        cp -rf "$t/$dir"/* $dir 2>/dev/null || true
    elif [[ $dir == include && -d "$t/inc" ]]; then
      cp -rf "$t/inc"/* $dir 2>/dev/null || true
    fi
  done

  mkdir -p "$name"
  tar c -O $FILES | (cd "$name"; tar xf -)
  [[ -d $SOLUTIONS_DIR ]] && cp -r $SOLUTIONS_DIR $name
  rm -rf src/* include/*
  if [[ -n $BUILD ]]; then 
    if ! (cd "$name"; ./build.sh reset 2>&1); then
      echo "Build failed for template $SOL"
      exit 1
    fi
    rm -rf "$name/build" 2>/dev/null || true
  fi
done

if [[ -n "$SAVE" && ! -e "$SAVE" ]]; then
  mkdir -p "$SAVE" 2>/dev/null || true
  mv $FILES "$SAVE"
fi

