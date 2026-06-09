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

#define GUEST_PHYS_START 0x50000000ULL
#define GUEST_SIZE       0x2000000ULL   // 32MB 示例

void main(void)
{
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
    
    /* 6. 创建VM实例 */
    hp_vm_id_t vm_id = hp_vm_create();
    if (vm_id == HP_INVALID_VM_ID) {
        platform_panic("Failed to create VM");
    }

    /* 7. 为VM添加内存区域（内部调用hp_stage2_map） */
    int32_t ret = hp_vm_add_memory_region(vm_id, 
                                          GUEST_PHYS_START,
                                          GUEST_SIZE,
                                          HP_MEM_READ | HP_MEM_WRITE | HP_MEM_EXEC);
    if (ret != 0) {
        platform_panic("Failed to add memory region");
    }
    // 映射 UART 给 VM，设备内存，读写，不可执行，不可缓存，不可共享
    //hp_vm_add_memory_region(vm_id, UART0_BASE, UART0_SIZE,
                        //HP_MEM_READ | HP_MEM_WRITE | HP_MEM_DEVICE);
    
    /* 8. 设置VM入口点 */
    hp_vm_set_entry(vm_id, GUEST_PHYS_START);
    
    /* ===== 第四阶段：创建和运行vCPU ===== */
    
    /* 9. 静态绑定：VM0 的 vCPU 运行在 CPU0，VM1 运行在 CPU1 */
    hp_vm_bind_vcpu(vm_id, 0U);

    platform_start_secondary_cpus();

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
