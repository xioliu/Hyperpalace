/**
 * @file src/main.c
 * @brief Hypervisor主入口，展示完整的调用链
 */
#include "hp_types.h"
#include "vm.h"
#include "hp_config.h"
#include "memory.h"
#include "arch_ops.h"
#include "platform.h"
#include "uart.h"
#include "sysregs.h"
#include "armv8_vm.h"
#include "armv8_vm_priv.h"
#include "vtimer.h"
#include "config.h"

extern const hp_vm_config_t vm_configs[];

void main(void)
{
    int32_t ret;
    uint32_t all_cpu_mask = 0U;
    /* ===== 第一阶段：系统级初始化 ===== */
    
    /* 1. 架构早期初始化（异常向量表等） */
    if ((g_arch_ops != NULL) && (g_arch_ops->early_init != NULL)) {
        g_arch_ops->early_init();
    }
    
    /* 2. 平台初始化（UART、中断控制器等） */
    platform_init();

    /* 3. 内存管理子系统初始化 */
    hp_memory_init();  /* 内部调用 g_arch_ops->mmu_early_init */
    
    /* 4. 架构后期初始化 */
    if ((g_arch_ops != NULL) && (g_arch_ops->late_init != NULL)) {
        g_arch_ops->late_init();
    }
    
    /* ===== 第二阶段：VM子系统初始化 ===== */
    
    /* 5. 初始化VM子系统静态池 */
    hp_vm_subsystem_init();
    
    /* ===== 第三阶段：创建和配置VM ===== */
    
    for (uint32_t i = 0U; i < vm_configs_count; i++) {
        all_cpu_mask |= vm_configs[i].cpu_mask;
        /* 创建 VM 实例 */
        hp_vm_id_t vm_id = hp_vm_create();
        if (vm_id == HP_INVALID_VM_ID) {
            platform_panic("Failed to create VM");
        }

        /* 添加 RAM 区域 */
        for (uint32_t r = 0U; r < vm_configs[i].num_ram_regions; r++) {
            int32_t ret = hp_vm_add_memory_region(
                vm_id,
                vm_configs[i].ram_regions[r].guest_pa,
                vm_configs[i].ram_regions[r].size,
                vm_configs[i].ram_regions[r].perm
            );
            if (ret != 0) {
                platform_panic("Failed to add RAM region");
            }
        }

        /* 添加设备区域 */
        for (uint32_t d = 0U; d < vm_configs[i].num_device_regions; d++) {
            int32_t ret = hp_vm_add_memory_region(
                vm_id,
                vm_configs[i].device_regions[d].guest_pa,
                vm_configs[i].device_regions[d].size,
                vm_configs[i].device_regions[d].perm
            );
            if (ret != 0) {
                platform_panic("Failed to add device region ret ");
                uart_puthex((uint64_t)ret);
                uart_puts("\n");
            }
        }

        /* 设置入口点 */
        hp_vm_set_entry(vm_id, vm_configs[i].entry);

        /* 遍历 CPU 掩码，为每个置位的 CPU 绑定一个 vCPU */
        uint32_t mask = vm_configs[i].cpu_mask;
        for (uint32_t cpu = 0U; cpu < HP_CONFIG_MAX_PCPUS; cpu++) {
            if (mask & (1U << cpu)) {
            ret = hp_vm_bind_vcpu(vm_id, cpu);
            if (ret != 0) {
                platform_panic("Failed to bind vCPU to CPU");
            }
        }
}

        /* 分配中断（例如虚拟定时器） */
        for (uint32_t irq_idx = 0U; irq_idx < vm_configs[i].num_irqs; irq_idx++) {
            ret = hp_vm_assign_interrupt(vm_id, vm_configs[i].irqs[irq_idx]);
            if (ret != 0) {
                platform_panic("Failed to assign interrupt");
            }
        }
    }

    platform_start_secondary_cpus(all_cpu_mask);

        // 创建 VM 后配置中断
    //hp_vm_assign_interrupt(vm_id, 27U);   // 物理定时器（示例）
    //hp_vm_assign_interrupt(vm_id, 33U);   // 某个 SPI 设备
    timer_init();

    /* 10. 启动vCPU（永不返回） */
    hp_vcpu_run_current();
    
    /* 不应该到达这里 */
    while (1) {
        __asm__ volatile("wfi");
    }
}
