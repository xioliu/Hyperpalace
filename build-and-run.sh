#! /bin/sh

# export PATH inclueding your cross compile tool
export PATH=$PATH:/home/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin
make run
qemu-system-aarch64 -machine virt,gic-version=3 -cpu cortex-a57 -machine virtualization=on -kernel minihyper.elf -nographic -S -s
