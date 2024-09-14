#ifndef __ARMV8_VM_H__
#define __ARMV8_VM_H__

#include <stdint.h>
#include <hyperpalace/vm.h>

/* ARMv8特定的VM私有数据 */
struct armv8_vm_arch {
    uint64_t vttbr_el2;     /* Stage-2页表基地址 */
    uint64_t *stage2_pgd;   /* Stage-2 PGD虚拟地址 */
};

/* ARMv8特定的vCPU私有数据 */
struct armv8_vcpu_arch {
    /* 保存的通用寄存器上下文 */
    uint64_t x[31];
    uint64_t sp_el0;
    uint64_t sp_el1;
    uint64_t elr_el2;
    uint64_t spsr_el2;
    
    /* 系统寄存器影子拷贝 */
    uint64_t vbar_el1;
    uint64_t sctlr_el1;
    /* ... 其他需要保存的寄存器 */
    
    bool running;
} __attribute__((aligned(16)));

/* 从公共结构体获取架构私有数据 */
static inline struct armv8_vm_arch* vm_to_arch(struct hp_vm *vm) {
    return (struct armv8_vm_arch*)vm->arch_private;
}

static inline struct armv8_vcpu_arch* vcpu_to_arch(struct hp_vcpu *vcpu) {
    return (struct armv8_vcpu_arch*)vcpu->arch_private;
}

#endif