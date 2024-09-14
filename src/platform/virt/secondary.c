#include "hp_types.h"
#include "vm.h"
#include "hp_config.h"
#include "platform.h"
#include "arch_ops.h"
//#include "uart.h"

/* 每个辅助 CPU 的专用栈 */
uint8_t secondary_stacks[HP_CONFIG_MAX_PCPUS - 1U][0x1000] __attribute__((aligned(16)));

/**
 * 辅助 CPU 入口
 * 从启动代码跳转而来（例如自旋检测释放后跳转）
 */
void secondary_cpu_entry(void)
{
    platform_init_secondary();

    /* 架构 per‑CPU 初始化（GIC CPU 接口、vGIC 等） */
    if ((g_arch_ops != NULL) && (g_arch_ops->early_init_secondary != NULL)) {
        g_arch_ops->early_init_secondary();
    }

    /* 直接运行绑定到当前物理 CPU 的 vCPU */
    hp_vcpu_run_current();

    /* 正常情况不会返回 */
    platform_panic("vCPU exited on secondary CPU");
}