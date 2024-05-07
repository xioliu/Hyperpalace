#include <stdint.h>
#include "aarch64.h"
#include "exceptions.h"
#include "gicv3.h"
#include "timer.h"
#include "uart.h"

#define TIMER_TIMEOUT   1

static uint32_t cntfrq;

void timer_handler(void)
{
    uint64_t current_cnt, next_cnt;
    
    uart_puts("timer_handler\n");    

    disable_cntv();
    gicd_clear_pending(TIMER_IRQ);

    current_cnt = raw_read_cntvct_el0();
    next_cnt = current_cnt + TIMER_TIMEOUT * cntfrq;
    raw_write_cntval_el0(next_cnt);

    enable_cntv();
}

void timer_init(void)
{
    uint64_t current_cnt, next_cnt;
    
    uart_puts("timer_init\n");   

    disable_cntv();
    cntfrq = raw_read_cntfrq_el0();
    current_cnt = raw_read_cntvct_el0();
    next_cnt = current_cnt + TIMER_TIMEOUT * cntfrq;
    raw_write_cntval_el0(next_cnt);

    gicd_irq_config(TIMER_IRQ, GIC_GICD_ICFGR_EDGE);
    gicd_set_priority(TIMER_IRQ, 0);
    gicd_set_target(TIMER_IRQ, 0x1); /*handled by cpu0*/
    gicd_clear_pending(TIMER_IRQ);
    gicd_enable_irq(TIMER_IRQ);

    enable_cntv();
    enable_irq();
}