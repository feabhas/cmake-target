#!/bin/bash
# QEMU Invocation for testing without graphics
# see https://www.qemu.org/docs/master/system/invocation.html

set -o nounset 
set -o errexit

QEMU_PATH="xpack-qemu-arm-2.8.0-9/bin/qemu-system-gnuarmeclipse"
QEMU_DIRS=". $HOME /opt"

TARGET=Application.elf
IMAGE=./build/debug/$TARGET

function usage {
  cat <<EOT
Usage: ${0#.*/} [elf-image]
  Launch QEMU from either local sub-directory, home or /opt directory.
  Filters startup diagnostic messages so output suitable for FileCheck testing
  Options:
    --help      -- this help information also -h -?
  Use Ctrl-A X to quit QEMU.
EOT
  exit 1
}

DIAG='-serial telnet:localhost:8888,server,nodelay'
SERIAL='-serial tcp::7777,server,nodelay'

for arg; do
  case "$arg" in
    *.ELF|*.elf)   IMAGE="$arg" ;;
    *)             echo "Unrecognised command argument: $arg" >&2
	           usage ;;
    esac
done

for dir in $QEMU_DIRS; do
  QEMU="$dir/$QEMU_PATH"
  [[ -x $QEMU ]] && break
  QEMU=
done

if [[ -z $QEMU ]]; then
  echo "Cannot find QEMU in standard locations: $QEMU_DIRS" >&2
  exit 1
fi

if [[ ! -f "$IMAGE" ]]; then
  echo "Missing image file: $IMAGE" >&2
  exit 1
fi

$QEMU --board Feabhas-WMS -d unimp,guest_errors \
      --semihosting-config enable=on,target=native \
      -nographic \
      --image "$IMAGE" 2>&1 \
      | sed -e '1,/^(qemu) diagnostic/d'

