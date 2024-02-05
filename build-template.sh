#!/bin/bash
# This script looks for exercise templates
# in a set of standard locations:

TEMPLATE_PATH="./exercises . ../exercises .. $HOME/exercises $HOME"
SOURCES="src include"
BACKUP=src.bak

# Creates self-contained workspaces for each template project by
# copying current workspace to subfolder named after the template.
# the workspace files are copied to a `project` subfolder.
# If there is only one template it is copied into the current workspace
# rather than create a new sufolder, current src/include files are saved
# to a `src.bak` folder.
# Templates folder must contain a src folder and optional include folder.

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
  echo "Workspace $PWD"
  TEMPLATE_DIR=
  for dir in $TEMPLATE_PATH ; do
    echo "Looking in $dir" >&2
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

# check for single template and copy to workspace itself
N=$(ls -l "$TEMPLATE_DIR" | grep '^d' | wc -l)
if (( $N == 1 )); then
  for name in "$TEMPLATE_DIR"/*; do
    if [[ -d "$name" ]]; then
        break
    fi
  done
  echo "Copying template $name to current workspace"
  for src in $SOURCES; do
    mkdir -p "$BACKUP/$src"
    mv "$src"/* "$BACKUP/$src" 2>/dev/null || true
    cp -r "$name/$src" . 2>/dev/null || true
  done
  exit 0
fi

SOLUTIONS_DIR=$(dirname $TEMPLATE_DIR)/solutions
  
[[ -d build ]] && rm -rf build

SCRIPT="$(basename $0)"
FILES=
for file in *; do
  case "$file" in
    exercises|templates|solutions|LICENSE*|"$SCRIPT") continue;;
  esac
  # check if previously generated template
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
      echo "Build failed for template $name"
      exit 1
    fi
    rm -rf "$name/build" 2>/dev/null || true
  fi
done

if [[ -n "$SAVE" && ! -e "$SAVE" ]]; then
  mkdir -p "$SAVE" 2>/dev/null || true
  mv $FILES "$SAVE"
fi

