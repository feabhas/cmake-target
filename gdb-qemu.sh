#!/bin/bash

arm-none-eabi-gdb -ex 'file build/debug/Application.elf' \
-ex 'target remote :1234' \
-ex 'set confirm off' \
-ex 'delete' \
-ex 'b main' \
-ex 'continue' \
-ex 'list'

