# Hyperpalace
A mini Hypervisor, designed as a bare-metal hypervisor for ARMv8 and RISC-V.

#How to run on QEMU
#For main branch, it works under EL1 and with gicv2.
qemu-system-aarch64 -machine virt -cpu cortex-a57 -kernel minihyper.elf -nographics -S -s

#For start-from-el2 branch, it works under EL2 and with gicv3.
qemu-system-aarch64 -machine virt,gic-version=3 -cpu cortex-a57 -machine virtualization=on -kernel minihyper.elf -nographic -S -s

