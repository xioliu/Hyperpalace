#ifndef ARMV8_VGIC_H
#define ARMV8_VGIC_H

#include <stdint.h>

/* 初始化当前 CPU 的虚拟 GIC 接口 */
void armv8_vgic_init(void);

/* 注入虚拟中断（使用默认优先级 0x80） */
void armv8_vgic_inject(uint32_t irq_id, uint8_t priority);

/* 处理虚拟化维护中断（由 GIC 模块调用） */
void armv8_vgic_handle_maintenance(uint32_t misr);

/* 在 VM 退出时保存/恢复状态（当前可空） */
void armv8_vgic_save_state(void);
void armv8_vgic_restore_state(void);

#endif