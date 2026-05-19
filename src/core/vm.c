#include "hp_types.h"
#include "vm.h"
#include "hp_config.h"
#include "memory.h"
#include "arch_ops.h"
#include "errno.h"
#include "string.h"

/* 静态池 */
static struct hp_vm g_vm_pool[HP_CONFIG_MAX_VMS];
static struct hp_vcpu g_vcpu_pool[HP_CONFIG_MAX_PCPUS];  /* 每个物理 CPU 最多一个 vCPU */
static hp_vcpu_id_t g_current_vcpu_per_cpu[HP_CONFIG_MAX_PCPUS];

/* 记录每个物理 CPU 是否已被绑定 */
static bool g_pcpu_bound[HP_CONFIG_MAX_PCPUS];

/* ========== 子系统初始化 ========== */
void hp_vm_subsystem_init(void)
{
    (void)memset(g_vm_pool, 0, sizeof(g_vm_pool));
    (void)memset(g_vcpu_pool, 0, sizeof(g_vcpu_pool));
    for (uint32_t i = 0U; i < HP_CONFIG_MAX_PCPUS; i++) {
        g_current_vcpu_per_cpu[i] = HP_INVALID_VCPU_ID;
        g_pcpu_bound[i] = false;
    }
}

/* ========== VM 管理 ========== */
hp_vm_id_t hp_vm_create(void)
{
    for (uint32_t i = 0U; i < HP_CONFIG_MAX_VMS; i++) {
        if (g_vm_pool[i].state == HP_VM_STATE_INVALID) {
            struct hp_vm *vm = &g_vm_pool[i];
            
            /* 分配 Stage-2 根页表 */
            uint64_t pgd_pa = hp_stage2_alloc_pgd();
            if (pgd_pa == 0ULL) {
                return HP_INVALID_VM_ID;
            }
            
            vm->id = (hp_vm_id_t)i;
            vm->state = HP_VM_STATE_CREATED;
            vm->pgd_pa = pgd_pa;
            vm->num_regions = 0U;
            vm->entry_point = 0ULL;
            for (uint32_t cpu = 0U; cpu < HP_CONFIG_MAX_PCPUS; cpu++) {
                vm->vcpu_per_cpu[cpu] = HP_INVALID_VCPU_ID;
            }
            
            /* 调用架构 VM 初始化 */
            if ((g_arch_ops != NULL) && (g_arch_ops->vm_init != NULL)) {
                int32_t ret = g_arch_ops->vm_init(vm->id);
                if (ret != 0) {
                    hp_stage2_free_pgd(pgd_pa);
                    vm->state = HP_VM_STATE_INVALID;
                    return HP_INVALID_VM_ID;
                }
            }
            return vm->id;
        }
    }
    return HP_INVALID_VM_ID;
}

void hp_vm_destroy(hp_vm_id_t vm_id)
{
    if (vm_id >= HP_CONFIG_MAX_VMS) return;
    struct hp_vm *vm = &g_vm_pool[vm_id];
    if (vm->state == HP_VM_STATE_INVALID) return;
    
    if ((g_arch_ops != NULL) && (g_arch_ops->vm_destroy != NULL)) {
        g_arch_ops->vm_destroy(vm_id);
    }
    
    hp_stage2_free_pgd(vm->pgd_pa);
    vm->state = HP_VM_STATE_INVALID;
}

int32_t hp_vm_add_memory_region(hp_vm_id_t vm_id, uint64_t guest_pa, uint64_t size, hp_mem_perm_t perm)
{
    if (vm_id >= HP_CONFIG_MAX_VMS) return HP_EINVAL;
    struct hp_vm *vm = &g_vm_pool[vm_id];
    if (vm->state == HP_VM_STATE_INVALID) return HP_EINVAL;
    if (vm->num_regions >= HP_CONFIG_MAX_MEM_REGIONS) return HP_ENOMEM;
    
    if (hp_memory_is_hyp_reserved(guest_pa, size)) {
        return HP_EPERM;
    }
    
    int32_t ret = hp_stage2_map(vm->pgd_pa, guest_pa, guest_pa, size, perm);
    if (ret != 0) return ret;
    
    vm->regions[vm->num_regions].guest_pa = guest_pa;
    vm->regions[vm->num_regions].size = size;
    vm->regions[vm->num_regions].perm = perm;
    vm->num_regions++;
    return 0;
}

void hp_vm_set_entry(hp_vm_id_t vm_id, uint64_t entry)
{
    if (vm_id < HP_CONFIG_MAX_VMS) {
        g_vm_pool[vm_id].entry_point = entry;
    }
}

uint64_t hp_vm_get_entry(hp_vm_id_t vm_id)
{
    if (vm_id < HP_CONFIG_MAX_VMS) {
        return g_vm_pool[vm_id].entry_point;
    }
    return 0ULL;
}

uint64_t hp_vm_get_pgd_pa(hp_vm_id_t vm_id)
{
    if (vm_id < HP_CONFIG_MAX_VMS) {
        return g_vm_pool[vm_id].pgd_pa;
    }
    return 0ULL;
}

/* ========== vCPU 静态绑定 ========== */
int32_t hp_vm_bind_vcpu(hp_vm_id_t vm_id, hp_pcpu_id_t pcpu_id)
{
    if (vm_id >= HP_CONFIG_MAX_VMS) return HP_EINVAL;
    if (pcpu_id >= HP_CONFIG_MAX_PCPUS) return HP_EINVAL;
    if (g_pcpu_bound[pcpu_id]) return HP_EBUSY;
    
    struct hp_vm *vm = &g_vm_pool[vm_id];
    if (vm->state == HP_VM_STATE_INVALID) return HP_EINVAL;
    
    /* 在 vCPU 池中分配一个槽位（直接用 pcpu_id 作为索引） */
    struct hp_vcpu *vcpu = &g_vcpu_pool[pcpu_id];
    if (vcpu->state != HP_VCPU_STATE_INVALID) {
        return HP_EBUSY;   /* 该 CPU 已绑定 */
    }
    
    vcpu->id = pcpu_id;                /* vCPU ID 与物理 CPU ID 一致 */
    vcpu->vm_id = vm_id;
    vcpu->pcpu_id = pcpu_id;
    vcpu->state = HP_VCPU_STATE_CREATED;
    
    /* 调用架构初始化 */
    if ((g_arch_ops != NULL) && (g_arch_ops->vcpu_init != NULL)) {
        int32_t ret = g_arch_ops->vcpu_init(vcpu->id);
        if (ret != 0) {
            vcpu->state = HP_VCPU_STATE_INVALID;
            return ret;
        }
    }
    
    /* 记录绑定关系 */
    vm->vcpu_per_cpu[pcpu_id] = vcpu->id;
    g_pcpu_bound[pcpu_id] = true;
    
    return 0;
}

/* 由各 CPU 启动时调用 */
void hp_vcpu_run_current(void)
{
    hp_pcpu_id_t cpu_id = hp_arch_get_current_cpu_id();
    if (cpu_id >= HP_CONFIG_MAX_PCPUS) {
        while (1) { __asm__ volatile("wfi"); }
    }
    
    struct hp_vcpu *vcpu = &g_vcpu_pool[cpu_id];
    if (vcpu->state != HP_VCPU_STATE_CREATED) {
        while (1) { __asm__ volatile("wfi"); }
    }
    
    vcpu->state = HP_VCPU_STATE_RUNNING;
    g_current_vcpu_per_cpu[cpu_id] = vcpu->id;
    
    /* 调用架构运行函数 */
    if ((g_arch_ops != NULL) && (g_arch_ops->vcpu_run != NULL)) {
        g_arch_ops->vcpu_run(vcpu->id);
    }
    
    /* 正常情况不会返回 */
    while (1) { __asm__ volatile("wfi"); }
}

hp_vcpu_id_t hp_vcpu_get_current(void)
{
    hp_pcpu_id_t cpu_id = hp_arch_get_current_cpu_id();
    if (cpu_id < HP_CONFIG_MAX_PCPUS) {
        return g_current_vcpu_per_cpu[cpu_id];
    }
    return HP_INVALID_VCPU_ID;
}

hp_vm_id_t hp_vcpu_get_vm_id(hp_vcpu_id_t vcpu_id)
{
    if (vcpu_id < HP_CONFIG_MAX_PCPUS) {
        return g_vcpu_pool[vcpu_id].vm_id;
    }
    return HP_INVALID_VM_ID;
}

void hp_vcpu_stop(hp_vcpu_id_t vcpu_id)
{
    if (vcpu_id < HP_CONFIG_MAX_PCPUS) {
        struct hp_vcpu *vcpu = &g_vcpu_pool[vcpu_id];
        vcpu->state = HP_VCPU_STATE_STOPPED;
        g_current_vcpu_per_cpu[vcpu->pcpu_id] = HP_INVALID_VCPU_ID;
        
        /* 可选：调用架构特定的清理 */
        if ((g_arch_ops != NULL) && (g_arch_ops->vcpu_stop != NULL)) {
            g_arch_ops->vcpu_stop(vcpu_id);
        }
    }
}

void hp_vcpu_stop_current(void)
{
    hp_pcpu_id_t cpu_id = hp_arch_get_current_cpu_id();
    hp_vcpu_id_t vcpu_id = hp_vcpu_get_current();
    if (vcpu_id < HP_CONFIG_MAX_PCPUS) {
        struct hp_vcpu *vcpu = &g_vcpu_pool[vcpu_id];
        vcpu->state = HP_VCPU_STATE_STOPPED;
        g_current_vcpu_per_cpu[cpu_id] = HP_INVALID_VCPU_ID;
        if ((g_arch_ops != NULL) && (g_arch_ops->vcpu_stop != NULL)) {
            g_arch_ops->vcpu_stop(vcpu_id);
        }
    }
}

int32_t hp_vm_assign_interrupt(hp_vm_id_t vm_id, uint32_t irq_id)
{
    if (vm_id >= HP_CONFIG_MAX_VMS) {
        return HP_EINVAL;
    }
    if (irq_id >= HP_IRQ_MAX) {
        return HP_EINVAL;
    }

    struct hp_vm *vm = &g_vm_pool[vm_id];
    if (vm->state == HP_VM_STATE_INVALID) {
        return HP_EINVAL;
    }

    /* 检查中断是否已被其他 VM 使用（静态分区下可禁止共享） */
    /* 遍历所有 VM 的位图，确保独占 */
    for (uint32_t i = 0U; i < HP_CONFIG_MAX_VMS; i++) {
        if (i != (uint32_t)vm_id && g_vm_pool[i].state != HP_VM_STATE_INVALID) {
            uint32_t word = irq_id / 32U;
            uint32_t bit = irq_id % 32U;
            if ((g_vm_pool[i].assigned_irqs[word] & (1U << bit)) != 0U) {
                return HP_EBUSY;   /* 中断已被占用 */
            }
        }
    }

    /* 记录中断 */
    uint32_t word = irq_id / 32U;
    uint32_t bit = irq_id % 32U;
    vm->assigned_irqs[word] |= (1U << bit);
    vm->num_assigned_irqs++;

    /* 调用架构层完成硬件配置（路由、使能） */
    if ((g_arch_ops != NULL) && (g_arch_ops->irq_assign != NULL)) {
        return g_arch_ops->irq_assign(vm_id, irq_id);
    }

    return 0;
}

hp_vcpu_id_t hp_vm_get_vcpu_id(hp_vm_id_t vm_id, hp_pcpu_id_t cpu_id)
{
    if (vm_id >= HP_CONFIG_MAX_VMS) {
        return HP_INVALID_VCPU_ID;
    }
    if (cpu_id >= HP_CONFIG_MAX_PCPUS) {
        return HP_INVALID_VCPU_ID;
    }

    struct hp_vm *vm = &g_vm_pool[vm_id];
    if (vm->state == HP_VM_STATE_INVALID) {
        return HP_INVALID_VCPU_ID;
    }

    return vm->vcpu_per_cpu[cpu_id];
}

bool hp_vm_is_irq_assigned(hp_vm_id_t vm_id, uint32_t irq_id)
{
    if (vm_id >= HP_CONFIG_MAX_VMS) return false;
    if (irq_id >= HP_IRQ_MAX) return false;

    struct hp_vm *vm = &g_vm_pool[vm_id];
    uint32_t word = irq_id / 32U;
    uint32_t bit = irq_id % 32U;
    return (vm->assigned_irqs[word] & (1U << bit)) != 0U;
}