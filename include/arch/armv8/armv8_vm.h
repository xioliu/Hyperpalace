#ifndef __ARMV8_VM_H__
#define __ARMV8_VM_H__

#include <stdint.h>
//#include <hyperpalace/vm.h>

/* ARMv8特定的VM私有数据 */
struct armv8_vm_arch {
    uint64_t vttbr_el2;     /* Stage-2页表基地址 */
    uint64_t *stage2_pgd;   /* Stage-2 PGD虚拟地址 */
};

struct arch_regs {
    uint64_t x[31];    // x0~x30 @ 0x00~0xF8
    uint64_t sp_el1;
    uint64_t elr_el2;  // 返回地址 @ 0xF8
    uint64_t spsr_el2; // 程序状态 @ 0x100
} __attribute__((aligned(16)));

/* ARMv8特定的vCPU私有数据 */
struct armv8_vcpu_arch {
    struct arch_regs regs;
    
    /* 系统寄存器影子拷贝 */
    uint64_t vbar_el1;
    uint64_t sctlr_el1;
    /* ... 其他需要保存的寄存器 */
    uint64_t vtimer_cval;
    uint64_t vtimer_ctl;
    uint64_t vtimer_offset;
    bool vtimer_pending;
    
    bool running;

    uint8_t stack[4096] __attribute__((aligned(16)));
} __attribute__((aligned(16)));

#endif