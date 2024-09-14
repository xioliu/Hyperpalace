#ifndef GICV3_H
#define GICV3_H

#include <stdint.h>
#include <stdbool.h>

/* 初始化 GICv3（主 CPU，需 distributor 权限） */
void gicv3_init(void);

/* 每个 CPU 的 GIC CPU 接口初始化 */
void gicv3_init_cpu(void);

/* 读取中断应答寄存器 */
uint32_t gicv3_read_iar(void);

/* 写入中断结束寄存器 */
void gicv3_write_eoir(uint32_t iar);

/* 释放中断（deactivate） */
void gicv3_write_dir(uint32_t irq_id);

/* 设置当前 CPU 的优先级掩码 */
void gicv3_set_pmr(uint8_t pmr);

/* 获取最高优先级挂起中断 */
uint32_t gicv3_get_highest_priority_pending(void);

/* 设置 SPI 的目标 CPU（亲和性路由） */
void gicv3_set_irq_target(uint32_t irq_id, uint8_t cpu_id);

/* 使能/禁用特定 IRQ */
void gicv3_enable_irq(uint32_t irq_id, bool enable);

/* 检查 IRQ 是否属于当前 VM（当前全直通，返回 true） */
bool gicv3_irq_belongs_to_vm(uint32_t irq_id);

/* 处理 Hypervisor 自身的中断（IPI、定时器等） */
void gicv3_handle_irq(void);

/* 维护中断处理入口（需要在向量表中调用） */
void gicv3_maintenance_handler(void);

uint8_t gicv3_get_physical_priority(uint32_t irq_id);

#endif /* GICV3_H */