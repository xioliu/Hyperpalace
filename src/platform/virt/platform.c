#include "hp_types.h"
#include "vm.h"
#include "hp_config.h"
#include "platform.h"
#include "uart.h"

#define PSCI_FN_CPU_ON  0xC4000003UL

/* 辅助 CPU 入口（定义在 secondary.c 中） */
extern void secondary_cpu_entry(void);

extern void secondary_start(void);

/* 用于辅助 CPU 启动的栈空间 */
extern uint8_t secondary_stacks[HP_CONFIG_MAX_PCPUS - 1U][0x1000];

/* 简单的自旋表：辅助 CPU 等待主 CPU 释放 */
volatile uint64_t cpu_release_addr[HP_CONFIG_MAX_PCPUS] __attribute__((aligned(64)));

static uint64_t psci_cpu_on(uint64_t target_cpu, uint64_t entry_addr, uint64_t context_id)
{
    uint64_t ret;
    __asm__ volatile (
        "mov x0, %[fn]\n"
        "mov x1, %[cpu]\n"
        "mov x2, %[entry]\n"
        "mov x3, %[ctx]\n"
        "smc #0\n"
        "mov %[ret], x0\n"
        : [ret] "=r"(ret)
        : [fn] "r"(PSCI_FN_CPU_ON), [cpu] "r"(target_cpu),
          [entry] "r"(entry_addr), [ctx] "r"(context_id)
        : "x0", "x1", "x2", "x3", "memory"
    );
    return ret;
}

void platform_init(void)
{
    /* 初始化 UART */
    uart_init();
    uart_puts("Hyperpalace: platform_init on CPU0\n");

    /* 辅助 CPU 释放地址初始化 */
    for (uint32_t i = 1U; i < HP_CONFIG_MAX_PCPUS; i++) {
        cpu_release_addr[i] = 0ULL;
    }
}

void platform_start_secondary_cpus(uint32_t cpu_mask)
{
    /* 从 CPU1 开始检查掩码 */
    for (uint32_t cpu = 1U; cpu < HP_CONFIG_MAX_PCPUS; cpu++) {
        if (cpu_mask & (1U << cpu)) {
            uint64_t entry = (uint64_t)secondary_start;
            uint64_t ret = psci_cpu_on(cpu, entry, 0);
            if (ret != 0) {
                uart_puts("PSCI CPU_ON failed for CPU ");
                uart_puthex(cpu);
                uart_puts("\n");
            }
        }
    }
}

void platform_init_secondary(void)
{
    /* 初始化 per‑CPU 的中断控制器（GIC CPU 接口） */
    /* 具体实现在 GIC 模块中 */

    /* 使能 FP/SIMD 供 Guest 使用（如果 Guest 需要） */
    uint64_t cpacr;
    __asm__ volatile("mrs %0, cpacr_el1" : "=r"(cpacr));
    cpacr |= (3U << 20);  /* FPEN = 1 (trap disabled) */
    __asm__ volatile("msr cpacr_el1, %0" : : "r"(cpacr));

    //uart_puts("Hyperpalace: secondary CPU initialized\n");
}

void platform_panic(const char *msg)
{
    uart_puts("PANIC: ");
    uart_puts(msg);
    uart_puts("\nSystem halted.\n");

    /* 停止所有活动 */
    while (1) {
        __asm__ volatile("wfi");
    }
}