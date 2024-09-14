#ifndef ARMV8_VM_PRIV_H
#define ARMV8_VM_PRIV_H

#include <hyperpalace/vm.h>
#include <hyperpalace/hp_config.h>

/* 每个 VM 的 ARMv8 私有数据 */
struct armv8_vm_arch {
    uint64_t vttbr_el2;             /* Stage-2 页表基址 (物理地址) */
    uint64_t *stage2_pgd_va;        /* Stage-2 PGD 虚拟地址 */
};

/* 每个 vCPU 的 ARMv8 私有数据 */
struct armv8_vcpu_arch {
    /* 保存的通用寄存器上下文 */
    uint64_t x[31];
    uint64_t sp_el0;
    uint64_t sp_el1;
    uint64_t elr_el2;
    uint64_t spsr_el2;
    /* 其他可能需要保存的系统寄存器 */
    uint64_t vbar_el1;
    uint64_t sctlr_el1;
    /* 标记 vCPU 是否正在运行（用于异常处理判断） */
    bool     running;
} __attribute__((aligned(16)));

/* 静态分配 ARMv8 私有数据池 */
extern struct armv8_vm_arch g_armv8_vm_arch_pool[HP_CONFIG_MAX_VMS];
extern struct armv8_vcpu_arch g_armv8_vcpu_arch_pool[HP_CONFIG_MAX_VCPUS];

/* 获取指定 ID 的架构私有指针 */
static inline struct armv8_vm_arch *armv8_get_vm_arch(hp_vm_id_t vm_id)
{
    if (vm_id < HP_CONFIG_MAX_VMS) {
        return &g_armv8_vm_arch_pool[vm_id];
    }
    return NULL;
}

static inline struct armv8_vcpu_arch *armv8_get_vcpu_arch(hp_vcpu_id_t vcpu_id)
{
    if (vcpu_id < HP_CONFIG_MAX_VCPUS) {
        return &g_armv8_vcpu_arch_pool[vcpu_id];
    }
    return NULL;
}

/* 检查物理地址是否属于 Hypervisor 保留区域（不可映射给 VM） */
bool armv8_is_hyp_reserved_region(uint64_t pa, uint64_t size);

void armv8_vcpu_run(hp_vcpu_id_t vcpu_id) HP_NORETURN;

#endif