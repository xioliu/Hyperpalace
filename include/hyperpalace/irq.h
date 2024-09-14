#ifndef HP_IRQ_H
#define HP_IRQ_H

#include <stdint.h>
#include <stdbool.h>

/* 最大物理中断号 */
#define HP_IRQ_MAX      1020U

/* 初始化中断子系统 */
void hp_irq_init(void);

/* 分发物理中断（由异常处理调用） */
void hp_irq_dispatch(uint32_t irq_id);

/* 查询指定中断是否属于当前运行的 VM */
bool hp_irq_is_for_current_vm(uint32_t irq_id);

#endif