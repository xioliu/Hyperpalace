#include <stdint.h>
#include "board-qemu.h"
#include "util.h"
#include "exceptions.h"
#include "gicv3.h"
#include "psw.h"
#include "timer.h"
#include "uart.h"

void gicd_init(void)
{
    uint32_t i, nr;
    uart_puts("gicd_init\n");

    *REG_GIC_GICD_CTLR = GICD_CTLR_DISABLE;

    nr = NUMBER(GIC_INT_MAX, GIC_GICD_INT_PER_REG);
    /*Disable interrupts*/
    for(i = 0; i < nr; i++)
        *REG_GIC_GICD_ICENABLER(i) = ~(uint32_t)0;
    /*Disable pending interrupts*/
    for(i = 0; i < nr; i++)
        *REG_GIC_GICD_ICPENDR(i) = ~(uint32_t)0;
    /*Deactive interrupts*/
    for(i = 0; i < nr; i++)
        *REG_GIC_GICD_ICACTIVER(i) = ~(uint32_t)0;

    /*Priority regs are reset to 0 which means highest priorities, so set to lowest during init*/
    nr = NUMBER(GIC_INT_MAX, GIC_GICD_INTPRIORITY_PER_REG);
    for(i = 0; i < nr; i++)
        *REG_GIC_GICD_IPRIORITYR(i) = ~(uint32_t)0;

    *REG_GIC_GICD_CTLR = GICD_CTLR_ENABLE;
}

void gicc_init(void)
{
    uart_puts("gicc_init\n");
    *REG_GIC_GICC_CTLR &= GICC_CTLR_DISABLE;

    /*Support lagest priority number*/
    *REG_GIC_GICC_PMR = 0xff;

    *REG_GIC_GICC_CTLR |= GICC_CTLR_ENABLE;
}

void gicd_irq_config(uint32_t irq, uint32_t cfg)
{
    uint32_t offset, value;

    offset = (irq % GIC_GICD_ICFGR_PER_REG) * GIC_GICD_ICFGR_BITS_PER_REG;
    value = *REG_GIC_GICD_ICFGR(irq / GIC_GICD_ICFGR_PER_REG);
    value &= ~((uint32_t)0x3 << offset);
    value |= (cfg << offset);
    *REG_GIC_GICD_ICFGR(irq / GIC_GICD_ICFGR_PER_REG) = value;
}

void gicd_set_priority(uint32_t irq, uint32_t pri)
{
    uint32_t offset, value;

    offset = (irq % GIC_GICD_INTPRIORITY_PER_REG) * GIC_GICD_INTPRIORITY_SIZE_PER_REG;
    value = *REG_GIC_GICD_IPRIORITYR(irq / GIC_GICD_INTPRIORITY_PER_REG);
    value &= ~((uint32_t)0xff << offset);
    value |= (pri << offset);
    *REG_GIC_GICD_IPRIORITYR(irq / GIC_GICD_INTPRIORITY_PER_REG) = value;
}

void gicd_set_target(uint32_t irq, uint32_t pe_nr)
{
    uint32_t offset, value;

    offset = (irq % GIC_GICD_ITARGETSR_PER_REG) * GIC_GICD_ITARGETSR_SIZE_PER_REG;
    value = *REG_GIC_GICD_ITARGETSR(irq / GIC_GICD_ITARGETSR_PER_REG);
    value &= ~((uint32_t)0xff << offset);
    value |= (pe_nr << offset);
    *REG_GIC_GICD_ITARGETSR(irq / GIC_GICD_ITARGETSR_PER_REG) = value;
}

void gicd_clear_pending(uint32_t irq)
{
    *REG_GIC_GICD_ICPENDR(irq / GIC_GICD_ICPENDR_PER_REG) |= 1 << (irq % GIC_GICD_ICPENDR_PER_REG);
}

void gicd_enable_irq(uint32_t irq)
{
    *REG_GIC_GICD_ISENABLER(irq / GIC_GICD_ISENABLER_PER_REG) |= 1 << (irq % GIC_GICD_ISENABLER_PER_REG);
}

void gicd_disable_irq(uint32_t irq)
{
    *REG_GIC_GICD_ICENABLER(irq / GIC_GICD_ICENABLER_PER_REG) |= 1 << (irq % GIC_GICD_ICENABLER_PER_REG);
}

/*Return 1 means a pending irq is found, otherwise 0*/
/*TODO: now iterate from irq 0 to max, see how to improve*/
static int gic_find_pending_irq(uint32_t *irq)
{
    uint32_t i;
    for(i = 0; i < GIC_INT_MAX; i++)
    {
        if(*REG_GIC_GICD_ISPENDR(i / GIC_GICD_ISPENDR_PER_REG) & (1 << (i % GIC_GICD_ISPENDR_PER_REG)))
        {
            *irq = i;
            return 1;
        }
    }
    return 0;
}

void gic_init(void)
{
    gicd_init();
    gicc_init();
}

void irq_handle(exception_t *excp __attribute__((unused)))
{
    uint32_t irq, ret;
    uint64_t psw;
    
    uart_puts("irq_handle\n");

    /*save psw*/
    psw_save(&psw);

    ret = gic_find_pending_irq(&irq);

    if(ret == 0)/*irq not found*/
        goto restore_psw;

    gicd_disable_irq(irq);
    gicd_clear_pending(irq);
    timer_handler();
    gicd_enable_irq(irq);

restore_psw:
    psw_restore(&psw);
}
