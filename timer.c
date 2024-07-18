#include <stdint.h>
#include "util.h"
#include "aarch64.h"
#include "exceptions.h"
#include "sysregs.h"
#include "gicv3.h"
#include "timer.h"
#include "uart.h"

#define TIMER_TIMEOUT   1

static uint32_t cntfrq;

void timer_handler(void)
{
    uint64_t current_cnt, next_cnt;
    
    //uart_puts("timer_handler\n");    

    disable_cntv();

    current_cnt = raw_read_cntpct_el0();
    next_cnt = current_cnt + TIMER_TIMEOUT * cntfrq;
    raw_write_cntval_el0(next_cnt);

    enable_cntv();
    
    uart_puts("\ncurrent_cnt =");
    uart_puthex(current_cnt);
    uart_puts("\n");
    uart_puts("\nnext_cnt =");
    uart_puthex(next_cnt);
    uart_puts("\n");
}

void timer_init(void)
{
    uint64_t current_cnt = 0, next_cnt = 0;
    
    uart_puts("timer_init\n");

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

    gicr_ppi_config(TIMER_IRQ, GIC_GICR_ICFGR_LEVEL);
    gicr_set_priority(TIMER_IRQ, 0xa0);
    gicr_clear_pending(TIMER_IRQ);
    gicr_enable_irq(TIMER_IRQ);
    
    enable_cntv();
    enable_irq();
}
