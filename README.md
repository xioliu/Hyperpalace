# Hyperpalace
A mini Hypervisor, designed as a bare-metal hypervisor for ARMv8 and RISC-V.

#How to run on QEMU
qemu-system-aarch64 -machine virt -cpu cortex-a53 -kernel minihyper.elf -nographics

