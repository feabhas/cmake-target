#!/bin/bash
# QEMU Invocation
# see https://www.qemu.org/docs/master/system/invocation.html

set -o nounset 
set -o errexit

TARGET=Application.elf
IMAGE=./build/debug/$TARGET

QEMU_PATH="xpack-qemu-arm-2.8.0-9/bin/qemu-system-gnuarmeclipse"
QEMU_DIRS=". $HOME /opt"

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
    --help     -- this help information also -h -?
  Use Ctrl-A X to quit QEMU if using --nographic.
EOT
  exit 1
}

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
    --nographic|-nographic)   
                   GRAPHIC=; DIAG= ;;
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

if [[ -z "$QEMU" ]]; then
  echo "Cannot find QEMU in standard locations: $QEMU_DIRS" >&2
  exit 1
fi

if [[ ! -f "$IMAGE" ]]; then
  echo "Missing image file: $IMAGE" >&2
  exit 1
fi

if [[ -z "$GRAPHIC" ]]; then
  OPTIONS="$OPTIONS -nographic --chardev file,path=/dev/tty,mux=on,id=c0 -mon chardev=c0 -serial chardev:c0"
fi

set -x
"$QEMU" --verbose --board Feabhas-WMS -d unimp,guest_errors \
        --semihosting-config enable=on,target=native \
        $OPTIONS $GDB $SERIAL $DIAG \
        --image "$IMAGE"

