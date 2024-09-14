#!/bin/sh
gdb-multiarch hyperpalace.elf -tui -ex "target remote :1234"