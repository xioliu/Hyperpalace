#ifndef HP_VM_H
#define HP_VM_H

/* VM 最大中断数（SPI 范围 32~1019） */
#define HP_CONFIG_MAX_IRQS_PER_VM   64U

/* VM 内存区域描述符 */
typedef struct {
    uint64_t guest_pa;
    uint64_t size;
    hp_mem_perm_t perm;
} hp_mem_region_t;

/* VM 控制块 */
struct hp_vm {
    hp_vm_id_t id;
    hp_vm_state_t state;
    hp_mem_region_t regions[HP_CONFIG_MAX_MEM_REGIONS];
    uint32_t num_regions;
    uint64_t entry_point;
    uint64_t pgd_pa;            /* Stage-2 根页表物理地址 */

    /* 静态分配的中断位图（bit 映射中断号） */
    uint32_t assigned_irqs[(HP_IRQ_MAX / 32U) + 1U];
    uint32_t num_assigned_irqs;

    /* 每个物理 CPU 上的 vCPU ID 数组，按 CPU ID 索引 */
    hp_vcpu_id_t vcpu_per_cpu[HP_CONFIG_MAX_PCPUS];
};

/* vCPU 控制块 */
struct hp_vcpu {
    hp_vcpu_id_t id;
    hp_vm_id_t vm_id;
    hp_pcpu_id_t pcpu_id;       /* 静态绑定的物理 CPU */
    hp_vcpu_state_t state;
    void *arch_private;         /* 架构私有上下文指针 */
};

/* ========== 子系统初始化 ========== */
void hp_vm_subsystem_init(void);

/* ========== VM 管理 ========== */
hp_vm_id_t hp_vm_create(void);
void hp_vm_destroy(hp_vm_id_t vm_id);
int32_t hp_vm_add_memory_region(hp_vm_id_t vm_id, uint64_t guest_pa, uint64_t size, hp_mem_perm_t perm);
void hp_vm_set_entry(hp_vm_id_t vm_id, uint64_t entry);

/* ========== vCPU 静态绑定管理 ========== */
/**
 * 为指定 VM 绑定一个 vCPU 到指定物理 CPU
 * 每个物理 CPU 只能绑定一次，通常在初始化阶段完成
 */
int32_t hp_vm_bind_vcpu(hp_vm_id_t vm_id, hp_pcpu_id_t pcpu_id);

/**
 * 启动当前物理 CPU 上绑定的 vCPU
 * 该函数由各 CPU 在初始化完成后独立调用，且不会返回
 */
void hp_vcpu_run_current(void) HP_NORETURN;

/**
 * 获取当前物理 CPU 上正在运行的 vCPU ID
 */
hp_vcpu_id_t hp_vcpu_get_current(void);

/**
 * 获取 vCPU 关联的 VM ID
 */
hp_vm_id_t hp_vcpu_get_vm_id(hp_vcpu_id_t vcpu_id);

/**
 * 获取 VM 的入口地址
 */
uint64_t hp_vm_get_entry(hp_vm_id_t vm_id);

/**
 * 获取 VM 的 Stage-2 根页表物理地址
 */
uint64_t hp_vm_get_pgd_pa(hp_vm_id_t vm_id);

void hp_vcpu_stop_current(void);

/* 分配中断给指定 VM */
int32_t hp_vm_assign_interrupt(hp_vm_id_t vm_id, uint32_t irq_id);

hp_vcpu_id_t hp_vm_get_vcpu_id(hp_vm_id_t vm_id, hp_pcpu_id_t cpu_id);

/* 检查指定 IRQ 是否已分配给该 VM */
bool hp_vm_is_irq_assigned(hp_vm_id_t vm_id, uint32_t irq_id);

void hp_vcpu_stop(hp_vcpu_id_t vcpu_id);

#endif /* HP_VM_H */