#! /bin/sh

# export PATH inclueding your cross compile tool
export PATH=$PATH:/realpath/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin
make run
qemu-system-aarch64 -machine virt -cpu cortex-a57 -kernel minihyper.elf -nographic -S -s
