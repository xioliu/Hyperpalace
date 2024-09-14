#ifndef HP_ARCH_OPS_H
#define HP_ARCH_OPS_H

#include <stdint.h>
#include <stdbool.h>

/* 前向声明 */
struct hp_vm;
struct hp_vcpu;

struct hp_arch_ops {
    /* VM 操作 */
    int32_t (*vm_init)(hp_vm_id_t vm_id);
    void    (*vm_destroy)(hp_vm_id_t vm_id);
    
    /* vCPU 操作 */
    int32_t (*vcpu_init)(hp_vcpu_id_t vcpu_id);
    void    (*vcpu_run)(hp_vcpu_id_t vcpu_id) __attribute__((noreturn));
    void    (*vcpu_stop)(hp_vcpu_id_t vcpu_id);
    void    (*vcpu_save_state)(hp_vcpu_id_t vcpu_id);
    uint32_t (*get_current_cpu_id)(void);

    /* IRQ 操作 */
    void (*irq_inject)(uint32_t irq_id, uint8_t priority);
    /* 获取物理中断优先级（可选，默认返回 0x80） */
    uint8_t (*get_physical_priority)(uint32_t irq_id);
    /* 分配中断号 */
    int32_t (*irq_assign)(hp_vm_id_t vm_id, uint32_t irq_id);

    /* MMU 操作 */
    void     (*mmu_early_init)(void);
    uint64_t (*mmu_alloc_pgd)(void);
    void     (*mmu_free_pgd)(uint64_t pgd_pa);
    int32_t  (*mmu_map)(uint64_t pgd_pa, uint64_t guest_pa, uint64_t host_pa,
                        uint64_t size, uint32_t perm);
    int32_t  (*mmu_unmap)(uint64_t pgd_pa, uint64_t guest_pa, uint64_t size);
    int32_t (*mmu_protect)(uint64_t pgd_pa, uint64_t guest_pa, uint64_t size,
                           uint32_t perm);
    bool     (*mmu_is_hyp_reserved)(uint64_t pa, uint64_t size);
    uint64_t (*va_to_pa)(const void *va);
    void*    (*pa_to_va)(uint64_t pa);
    
    /* 架构初始化 */
    void (*early_init)(void);
    void (*late_init)(void);
    void (*early_init_secondary)(void);  /* 辅助 CPU 的架构初始化 */
};

extern const struct hp_arch_ops *g_arch_ops;

/* 架构无关的当前 CPU ID 获取函数 */
static inline uint32_t hp_arch_get_current_cpu_id(void)
{
    if ((g_arch_ops != NULL) && (g_arch_ops->get_current_cpu_id != NULL)) {
        return g_arch_ops->get_current_cpu_id();
    }
    return 0U;   /* 默认返回 CPU0 */
}

#endif