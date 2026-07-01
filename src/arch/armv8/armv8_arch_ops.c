#include "hp_types.h"
#include "vm.h"
#include "hp_config.h"
#include "memory.h"
#include "arch_ops.h"
#include "errno.h"
#include "arch_ops.h"
#include "armv8_vm.h"
#include "armv8_vm_priv.h"
#include "armv8_mmu.h"
#include "platform.h"
#include "gicv3.h"
#include "sysregs.h"
#include "vgic.h"

/* 声明所有本地函数 */
extern int32_t armv8_vm_init(hp_vm_id_t vm_id);
extern void armv8_vm_destroy(hp_vm_id_t vm_id);
extern int32_t armv8_vm_map_memory(hp_vm_id_t vm_id, uint64_t guest_pa, uint64_t size, hp_mem_perm_t perm);
extern int32_t armv8_vcpu_init(hp_vcpu_id_t vcpu_id);
extern void armv8_vcpu_run(hp_vcpu_id_t vcpu_id);
extern void armv8_vcpu_stop(hp_vcpu_id_t vcpu_id);
extern void armv8_vcpu_save_state(hp_vcpu_id_t vcpu_id);
extern void armv8_mmu_early_init(void);
extern uint64_t armv8_mmu_alloc_pgd(void);
extern void armv8_mmu_free_pgd(uint64_t pgd_pa);
extern int32_t armv8_mmu_map(uint64_t pgd_pa, uint64_t guest_pa, uint64_t host_pa,
                             uint64_t size, uint32_t perm);
extern int32_t armv8_mmu_unmap(uint64_t pgd_pa, uint64_t guest_pa, uint64_t size);
extern bool armv8_mmu_is_hyp_reserved(uint64_t pa, uint64_t size);
extern uint64_t armv8_va_to_pa(const void *va);
extern void* armv8_pa_to_va(uint64_t pa);
extern void armv8_early_init(void);
extern void armv8_late_init(void);
extern uint32_t armv8_get_current_cpu_id(void);
extern void armv8_vgic_inject(uint32_t irq_id, uint8_t priority);
extern int32_t armv8_mmu_protect(uint64_t pgd_pa, uint64_t guest_pa, uint64_t size, uint32_t perm);

void armv8_early_init(void)
{
    /* 初始化GIC等（可调用你已有的GICv3初始化） */
    gicv3_init();
}

void armv8_late_init(void)
{
    /* 配置HCR_EL2等 */
    uint64_t hcr = read_hcr_el2();
    hcr |= HCR_VM_BIT;
    hcr |= HCR_RW_BIT;
    //hcr |= HCR_IMO_BIT;//中断直通guest处理是不能置位的
    hcr |= HCR_FMO_BIT;
    hcr |= HCR_AMO_BIT;
    //hcr |= HCR_TSC_BIT;
    //hcr |= (0x1 << 13);//WFI trap
    write_hcr_el2(hcr);

    /* 主 CPU 的 per‑CPU 虚拟 GIC 初始化 */
    armv8_vgic_init();
}

/* 辅助 CPU 架构初始化 */
void armv8_early_init_secondary(void)
{
    /* 1. 使能当前 CPU 的 GIC CPU 接口 */
    //gicv3_init_cpu();
    armv8_early_init();

    /* 2. 使能 FP/SIMD 访问（Guest 可能需要） */
    uint64_t cpacr;
    __asm__ volatile("mrs %0, cpacr_el1" : "=r"(cpacr));
    cpacr |= (3U << 20);  /* FPEN = 1 */
    __asm__ volatile("msr cpacr_el1, %0" : : "r"(cpacr));

    /* 3. 虚拟 GIC 每 CPU 初始化 */
    //armv8_vgic_init();
    armv8_late_init();
}

/* 获取当前 CPU ID */
uint32_t armv8_get_current_cpu_id(void)
{
    uint64_t mpidr = read_mpidr_el1();
    return (uint32_t)(mpidr & 0xFFU);
}

/* 获取物理优先级：从 GIC Distributor 读取 */
uint8_t armv8_get_physical_priority(uint32_t irq_id)
{
    return gicv3_get_physical_priority(irq_id);
}

int32_t armv8_irq_assign(hp_vm_id_t vm_id, uint32_t irq_id)
{
    /* 获取 VM 绑定的物理 CPU 列表，这里假设一个 VM 至少一个 vCPU，
       为简单起见，将中断路由到该 VM 的第一个绑定的物理 CPU */
    hp_pcpu_id_t target_cpu = HP_INVALID_CPU_ID;
    for (uint32_t cpu = 0U; cpu < HP_CONFIG_MAX_PCPUS; cpu++) {
        if (hp_vm_get_vcpu_id(vm_id, cpu) != HP_INVALID_VCPU_ID) {
            target_cpu = cpu;
            break;
        }
    }
    if (target_cpu == HP_INVALID_CPU_ID) {
        return HP_EINVAL;   /* 该 VM 未绑定 vCPU */
    }

    /* 设置中断亲和性到目标 CPU */
    gicv3_set_irq_target(irq_id, target_cpu);

    /* 使能中断 */
    gicv3_enable_irq(irq_id, true);

    if (irq_id >= 32U) {   /* SPI 中断，PPI 和 SGI 已在 gicr 中处理 */
        uint32_t reg = irq_id / 32U;
        uint32_t bit = irq_id % 32U;
        uint32_t igroupr = mmio_read32(GICD_BASE + GICD_IGROUPR(reg));
        igroupr |= (1U << bit);
        mmio_write32(GICD_BASE + GICD_IGROUPR(reg), igroupr);
    }

    return 0;
}

const struct hp_arch_ops g_arch_ops_armv8 = {
    .early_init               = armv8_early_init,
    .late_init                = armv8_late_init,
    .early_init_secondary     = armv8_early_init_secondary,

    .get_current_cpu_id       = armv8_get_current_cpu_id,

    .vm_init                  = armv8_vm_init,
    .vm_destroy               = armv8_vm_destroy,

    .vcpu_init                = armv8_vcpu_init,
    .vcpu_run                 = armv8_vcpu_run,
    .vcpu_stop                = armv8_vcpu_stop,
    .vcpu_save_state          = armv8_vcpu_save_state,

    .irq_inject               = armv8_vgic_inject,
    .get_physical_priority    = armv8_get_physical_priority,
    .irq_assign               = armv8_irq_assign,

    .mmu_early_init           = armv8_mmu_early_init,
    .mmu_alloc_pgd            = armv8_mmu_alloc_pgd,
    .mmu_free_pgd             = armv8_mmu_free_pgd,
    .mmu_map                  = armv8_mmu_map,
    .mmu_unmap                = armv8_mmu_unmap,
    .mmu_protect              = armv8_mmu_protect,
    .mmu_is_hyp_reserved      = armv8_mmu_is_hyp_reserved,
    .va_to_pa                 = armv8_va_to_pa,
    .pa_to_va                 = armv8_pa_to_va,
};

const struct hp_arch_ops *g_arch_ops = &g_arch_ops_armv8;