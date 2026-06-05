#ifndef VTIMER_H
#define VTIMER_H

#include <stdint.h>
#include <stdbool.h>

struct armv8_vcpu_arch;

/* 初始化 vCPU 虚拟定时器 */
void vtimer_init(struct armv8_vcpu_arch *arch);

/* 处理陷阱访问，返回 true 表示已处理 */
bool vtimer_handle_trap(struct armv8_vcpu_arch *arch, uint64_t esr);

/* 检查定时器条件，必要时注入中断（可在每次 vm exit 时调用） */
void vtimer_check_inject(struct armv8_vcpu_arch *arch);

void vtimer_set_cval(struct armv8_vcpu_arch *arch, uint64_t cval);

#endif