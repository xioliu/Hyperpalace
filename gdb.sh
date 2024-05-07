#! /bin/sh

# export PATH inclueding your cross compile tool
export PATH=$PATH:/realpath/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin
aarch64-none-elf-gdb minihyper.elf --tui
#target remote :1234
