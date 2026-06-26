#include "hp_types.h"
#include "vm.h"
#include "hp_config.h"
#include "armv8_vm.h"
#include "armv8_mmu_priv.h"
#include "platform.h"
#include "arch_ops.h"
#include "sysregs.h"
#include "vtimer.h"
#include "uart.h"

/* 每个辅助 CPU 的专用栈 */
uint8_t secondary_stacks[HP_CONFIG_MAX_PCPUS - 1U][0x1000] __attribute__((aligned(16)));

/**
 * 辅助 CPU 入口
 * 从启动代码跳转而来（例如自旋检测释放后跳转）
 */
void secondary_cpu_entry(void)
{
    /* 从主核复制 VTCR_EL2 和 MAIR_EL2 */
    // 假设主核的配置存储在全局变量中
    write_vtcr_el2(g_armv8_mmu_ctx.vtcr_el2);
    write_mair_el2(g_armv8_mmu_ctx.mair_el2);

    /* 平台 per-CPU 初始化（包括 GIC CPU 接口） */
    platform_init_secondary();

    /* 架构 per-CPU 初始化 */
    if ((g_arch_ops != NULL) && (g_arch_ops->early_init_secondary != NULL)) {
        g_arch_ops->early_init_secondary();
    }

    timer_init_cpu1();

    /* 直接运行绑定到当前物理 CPU 的 vCPU */
    hp_vcpu_run_current();

    /* 正常情况不会返回 */
    uart_puts("vCPU exited on secondary CPU");
}