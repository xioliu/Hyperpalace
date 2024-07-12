#include <stdint.h>
#include "util.h"
#include "aarch64.h"
#include "exceptions.h"
#include "gicv3.h"
#include "timer.h"
#include "uart.h"

#define TIMER_TIMEOUT   1000

static uint32_t cntfrq;

void timer_handler(void)
{
    uint64_t current_cnt, next_cnt;
    
    uart_puts("timer_handler\n");    

    disable_cntv();

    current_cnt = raw_read_cntpct_el0();
    next_cnt = current_cnt + TIMER_TIMEOUT * cntfrq;
    raw_write_cntval_el0(next_cnt);

    enable_cntv();
}

void timer_init(void)
{
    uint64_t current_cnt = 0, next_cnt = 0;
    
    uart_puts("timer_init\n");

    uart_puts("CurrentEL = ");
	uart_puthex(raw_read_current_el());

    uart_puts("\nDAIF-1 = ");
	uart_puthex(raw_read_daif());

    disable_cntv();
    cntfrq = raw_read_cntfrq_el0();
    current_cnt = raw_read_cntpct_el0();
    next_cnt = current_cnt + TIMER_TIMEOUT * cntfrq;
    raw_write_cntval_el0(next_cnt);
    uart_puts("\ncntfrq = ");
    uart_puthex(cntfrq);
    uart_puts("\ncurrent_cnt = ");
    uart_puthex(current_cnt);
    uart_puts("\nnext_cnt = ");
    uart_puthex(next_cnt);

    gicd_irq_config(TIMER_IRQ, GIC_GICD_ICFGR_LEVEL);
    gicd_set_priority(TIMER_IRQ, 0);
    gicd_set_target(TIMER_IRQ, 0x1); /*handled by cpu0*/
    gicd_clear_pending(TIMER_IRQ);
    gicd_enable_irq(TIMER_IRQ);

    enable_cntv();
    enable_irq();
    
    uart_puts("\nDAIF-2 = ");
	uart_puthex(raw_read_daif());
	uart_puts("\n");
}
