#include "hp_types.h"
#include "vm.h"
#include "exceptions.h"
#include "sysregs.h"
#include "platform.h"
#include "uart.h"
#include "vtimer.h"
#include "armv8_vm.h"
#include "armv8_vm_priv.h"
#include "vgic.h"
#include "gicv3.h"

/* 虚拟定时器中断号 */
#define VIRT_TIMER_PPI       27U

void vtimer_init(struct armv8_vcpu_arch *arch)
{
    arch->vtimer_cval = 0;
    arch->vtimer_ctl = 0;
    arch->vtimer_offset = 0;
    arch->vtimer_pending = false;
}

/* 获取当前虚拟时间 = 物理时间 + 偏移 */
static uint64_t vtimer_current_time(struct armv8_vcpu_arch *arch)
{
    return read_cntpct_el0() + arch->vtimer_offset;
}

bool vtimer_handle_trap(struct armv8_vcpu_arch *arch, uint64_t esr)
{
    uart_puts("vtimer trap, esr=");
    uart_puthex(esr);
    uart_puts("\n");
    /* 检查是否为系统寄存器陷阱 (EC=0x18) */
    uint32_t ec = (esr >> 26) & 0x3F;
    if (ec != 0x18U) return false;

    /* 获取系统寄存器编码 */
    uint16_t sys_reg = (esr >> 5) & 0x7FFU;
    //bool is_read = (esr & (1U << 0)) != 0;   /* ESR_EL2.ISV 指示读/写？实际上需要根据 ISS 分析 */
    /* 更准确的判断：位[0] 方向：0=write, 1=read */
    bool is_write = ((esr & 0x1U) == 0U);

    switch (sys_reg) {
        case 0x1E00://SREG_CNTV_CTL_EL0
            if (is_write) {
                /* 写入 CTL: x0 通常包含要写的值，需从异常帧获取源寄存器 */
                /* 简化：从 arch->x[0] 获得写入的值（假设源寄存器是 x0） */
                uint32_t val = (uint32_t)arch->regs.x[0];
                arch->vtimer_ctl = val;
            } else {
                /* 读取 CTL，返回当前状态 */
                arch->regs.x[0] = arch->vtimer_ctl;
            }
            break;

        case 0x1E02://SREG_CNTV_CVAL_EL0
            if (is_write) {
                arch->vtimer_cval = arch->regs.x[0];
                /* 清除挂起中断 */
                arch->vtimer_pending = false;
            } else {
                arch->regs.x[0] = arch->vtimer_cval;
            }
            break;

        case 0x1E01://SREG_CNTV_TVAL_EL0
            if (is_write) {
                uint32_t tval = (uint32_t)arch->regs.x[0];
                /* 新的 CVAL = 当前虚拟时间 + tval */
                arch->vtimer_cval = vtimer_current_time(arch) + tval;
                arch->vtimer_pending = false;
            } else {
                /* 读取 TVAL = CVAL - 当前虚拟时间 */
                uint64_t cur = vtimer_current_time(arch);
                if (arch->vtimer_cval > cur)
                    arch->regs.x[0] = arch->vtimer_cval - cur;
                else
                    arch->regs.x[0] = 0;
            }
            break;

        case 0x1E03://SREG_CNTVCT_EL0
            if (!is_write) {
                arch->regs.x[0] = vtimer_current_time(arch);
            }
            break;

        default:
            return false;   // 未处理
    }

    /* 模拟完成后，递增 ELR 以跳过陷阱指令 */
    //arch->elr_el2 += 4U;
    return true;
}

void vtimer_check_inject(struct armv8_vcpu_arch *arch)
{
    /* 检查定时器使能且未屏蔽 */
    uart_puts("vtimer: ctl=");
    uart_puthex(arch->vtimer_ctl);
    uart_puts(" cval=");
    uart_puthex(arch->vtimer_cval);
    uart_puts(" now=");
    uart_puthex(read_cntpct_el0());
    uart_puts("\n");
    if ((arch->vtimer_ctl & 0x1U) == 0 || (arch->vtimer_ctl & 0x2U) != 0)
        return;

    uint64_t now = vtimer_current_time(arch);
    uart_puts("vtimer_check: now=");
    uart_puthex(now);
    uart_puts(" cval=");
    uart_puthex(arch->vtimer_cval);
    uart_puts("\n");
    if (arch->vtimer_cval <= now && !arch->vtimer_pending) {
        /* 触发虚拟定时器中断 */
        arch->vtimer_pending = true;
        armv8_vgic_inject(VIRT_TIMER_PPI, 0x80);   // 注入虚拟 PPI 27
    }
}

void vtimer_set_cval(struct armv8_vcpu_arch *arch, uint64_t cval)
{
    arch->vtimer_cval = cval;
    arch->vtimer_ctl = 1;   // enable, unmask
    arch->vtimer_pending = false;
}