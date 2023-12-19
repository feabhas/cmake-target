#!/bin/bash
# QEMU Invocation
# see https://www.qemu.org/docs/master/system/invocation.html

TARGET=Application.elf
IMAGE=./build/debug/$TARGET

function usage {
  cat <<EOT
Usage: ${0#.*/} [qemu-options] [diag] [gdb] [serial] [elf-image]
  Launch QEMU from either local sub-directory, home or /opt directory.
  Starts up with diagnostic port 8888 open ready for remote GUI.
  Options:
    serial        - start USART3 server on port 7777
    gdb           - start GDB server on port 1234
    --nographic   - switch off graphic support
    --sdl2        - use built-in graphics instead of remote port 8888
  Use Ctrl-A X to quit QEMU if using --nographic.
EOT
  exit 1
}

QEMU_PATH="xpack-qemu-arm-2.8.0-9/bin/qemu-system-gnuarmeclipse"
QEMU_DIRS=". $HOME /opt"

DIAG='-serial telnet:localhost:8888,server,nodelay'
GRAPHIC=
GDB=
SERIAL=
OPTIONS=

for arg; do
  case "$arg" in
    --help|-h|-\?) usage ;;
    gdb)           GDB='-S -gdb tcp::1234' ;;
    serial)        SERIAL='-serial tcp::7777,server,nodelay' ;;
    *.ELF|*.elf)   IMAGE="$arg" ;;
    --nographic)   GRAPHIC=; DIAG= ;;
    --sdl2)        GRAPHIC=y; DIAG= ;;
    -*)            OPTIONS="$OPTIONS $arg" ;;
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

if [[ -z "$GRAPHIC" ]]; then
  OPTIONS="$OPTIONS -nographic"
fi

set -x
$QEMU --verbose --board Feabhas-WMS -d unimp,guest_errors \
      --semihosting-config enable=on,target=native \
      $OPTIONS $GDB $SERIAL $DIAG \
      --image $IMAGE

