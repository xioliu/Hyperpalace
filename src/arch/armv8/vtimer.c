#include "hp_types.h"
#include "vm.h"
#include "exceptions.h"
#include "sysregs.h"
#include "platform.h"
#include "uart.h"
#include "armv8_vm.h"
#include "armv8_vm_priv.h"
#include "vgic.h"
#include "gicv3.h"
#include "vtimer.h"

/* 定时器中断处理函数 */
void virt_timer_interrupt_handler(struct arch_regs* regs)
{
    struct armv8_vcpu_arch* vcpu = (struct armv8_vcpu_arch*)regs;

    /* 确认是虚拟定时器中断 */
    uint64_t cntv_ctl = read_cntv_ctl_el0();
    if (!(cntv_ctl & (1 << 2))) { // ISTATUS位
        return;
    }
    
    /* 标记虚拟中断待注入 */
    vcpu->vtimer_pending = true;
    static uint64_t count = 0;
    uart_puts("vtimer irq handler count \n");
    uart_puthex(++count);
    uart_puts("\n");

    // 测试代码，guest timer重置以测试周期性中断
    uint64_t now = read_cntvct_el0();
    uint64_t new_cval = now + 62500000; // 62.5MHz频率，1秒
    write_cntv_cval_el0(new_cval);
    isb();
}

/* 定时器中断属于PPI，由GICR配置 */
void timer_init(void)
{
    uart_puts("vtimer_init\n");

    gicr_ppi_config(VTIMER_IRQ, 0);//GIC_GICR_ICFGR_LEVEL
    gicr_set_priority(VTIMER_IRQ, 0xa0);
    gicr_clear_pending(VTIMER_IRQ);
    gicr_enable_irq(VTIMER_IRQ);
}

#if 0
void timer_handler(void)
{
    uint64_t cntfrq, current_cnt, next_cnt;
    uart_puts("ptimer irq handler\n");    

    disable_cntv();
    cntfrq = raw_read_cntfrq_el0();
    current_cnt = raw_read_cntpct_el0();
    next_cnt = current_cnt + 1 * cntfrq;//enable again
    raw_write_cntval_el2(next_cnt);
    enable_cntv();
}


// ==============================
// 初始化 EL2 物理定时器，每 1 秒产生一次中断
// ==============================
void el2_phys_timer_init(void)
{
    uint64_t cntfrq, current_cnt, next_cnt;
    uart_puts("ptimer_init\n");

    disable_cntv();
    cntfrq = raw_read_cntfrq_el0();
    current_cnt = raw_read_cntpct_el0();
    next_cnt = current_cnt + 1 * cntfrq;//1s
    raw_write_cntval_el2(next_cnt);
    enable_cntv();

    gicr_ppi_config(PTIMER_IRQ, 0);//GIC_GICR_ICFGR_LEVEL
    gicr_set_priority(PTIMER_IRQ, 0xa0);
    gicr_clear_pending(PTIMER_IRQ);
    gicr_enable_irq(PTIMER_IRQ);
}
#endif