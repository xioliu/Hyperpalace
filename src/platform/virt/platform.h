#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>

#define GICD_BASE  0x08000000ULL
#define GICR_BASE  0x080A0000ULL
#define GICC_BASE  0x08010000ULL   /* QEMU virt 兼容的 legacy 映射 */

/* 平台初始化（主 CPU） */
void platform_init(void);

/* 启动所有辅助 CPU */
void platform_start_secondary_cpus(void);

/* 辅助 CPU per‑CPU 初始化 */
void platform_init_secondary(void);

/* 平台级 panic，打印消息并停止 */
void platform_panic(const char *msg);

/* UART 基础打印（用于早期调试） */
void uart_puts(const char *s);
void uart_puthex(uint64_t val);

#endif /* PLATFORM_H */