#include "hp_types.h"
#include "vm.h"
#include "hp_config.h"
#include "platform.h"
#include "uart.h"

/* 辅助 CPU 入口（定义在 secondary.c 中） */
extern void secondary_cpu_entry(void);

/* 用于辅助 CPU 启动的栈空间 */
extern uint8_t secondary_stacks[HP_CONFIG_MAX_PCPUS - 1U][0x1000];

/* QEMU virt 平台内存布局常量 */
#define PSCI_BASE       0x09000000ULL
#define VIRT_UART_BASE  0x09000000ULL
#define GIC_DIST_BASE   0x08000000ULL
#define GIC_CPU_BASE    0x08010000ULL

/* 简单的自旋表：辅助 CPU 等待主 CPU 释放 */
volatile uint64_t cpu_release_addr[HP_CONFIG_MAX_PCPUS] __attribute__((aligned(64)));

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

void platform_start_secondary_cpus(void)
{
    for (uint32_t cpu = 1U; cpu < HP_CONFIG_MAX_PCPUS; cpu++) {
        /* 设置辅助 CPU 的启动地址（物理地址） */
        cpu_release_addr[cpu] = (uint64_t)secondary_cpu_entry;

        /* 内存屏障确保写入对辅助 CPU 可见 */
        __asm__ volatile("dsb ishst");
        __asm__ volatile("sev");  /* 发送事件唤醒 WFE */
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

    uart_puts("Hyperpalace: secondary CPU initialized\n");
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