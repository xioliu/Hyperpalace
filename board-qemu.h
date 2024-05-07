#ifndef __BOARD_H
#define __BOARD_H

#define QEMU_VIRT_GIC_BASE          0x08000000
#define QEMU_VIRT_GIC_INT_MAX       64
#define QEMU_VIRT_GIC_PRIO_MAX      16

#define QEMU_VIRT_GIC_INTNO_SGIO    0
#define QEMU_VIRT_GIC_INTNO_PPIO    16
#define QEMU_VIRT_GIC_INTNO_SPIO    32

#endif