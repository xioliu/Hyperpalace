#include <stdint.h>
#include "board-qemu.h"
#include "util.h"
#include "exceptions.h"
#include "sysregs.h"
#include "gicv3.h"
#include "psw.h"
#include "timer.h"
#include "uart.h"
#include "aarch64.h"

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

    *REG_GIC_GICD_CTLR = GICD_CTLR_ARE_NS_BIT | GICD_CTLR_ENA_BIT;
}

void gicr_init(void)
{
    uint32_t i, nr;
    uart_puts("gicr_init\n");

    *REG_GIC_GICR_WAKER &= ~GICR_WAKER_ProcessorSleep_BIT;
    while(*REG_GIC_GICR_WAKER & GICR_WAKER_ChildrenASleep_BIT);

    nr = NUMBER(GIC_INT_MAX, GIC_GICR_INT_PER_REG);

    *REG_GIC_GICR_IGROUPR0 = ~0U;
    *REG_GIC_GICR_ICENABLER0 = ~0U;
    *REG_GIC_GICR_ICPENDR0 = ~0U;
    *REG_GIC_GICR_ICACTIVER0 = ~0U;
    for(i = 0; i < nr; i++)
        *REG_GIC_GICR_IPRIORITYR(i) = ~0U;
}

void gicc_init(void)
{
    uart_puts("gicc_init\n");

    uint32_t lr_num = gich_num_lrs();
    sysreg_icc_sre_el2_write(ICC_SRE_SRE_BIT | ICC_SRE_ENB_BIT);
    ISB();
    
    for (uint32_t i = 0; i < lr_num; i++) {
        gich_write_lr(i, 0);
    }

    sysreg_icc_pmr_el1_write(0xff);
    sysreg_icc_bpr1_el1_write(0x0);
    sysreg_icc_ctlr_el1_write(ICC_CTLR_EOIMode_BIT);
    sysreg_ich_hcr_el2_write(sysreg_ich_hcr_el2_read() | ICH_HCR_LRENPIE_BIT);
    sysreg_icc_igrpen1_el1_write(ICC_IGRPEN_EL1_ENB_BIT);
}

uint32_t gich_num_lrs(void)
{
    return ((sysreg_ich_vtr_el2_read() & ICH_VTR_MSK) >> ICH_VTR_OFF) + 1;
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
    value |= (pe_nr << offset);//pe_nr
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

void gicr_set_priority(uint32_t irq, uint32_t pri)
{
    uint32_t offset, value;

    offset = (irq % GIC_GICR_INTPRIORITY_PER_REG) * GIC_GICR_INTPRIORITY_SIZE_PER_REG;
    value = *REG_GIC_GICR_IPRIORITYR(irq / GIC_GICR_INTPRIORITY_PER_REG);
    value &= ~((uint32_t)0xff << offset);
    value |= (pri << offset);
    *REG_GIC_GICR_IPRIORITYR(irq / GIC_GICR_INTPRIORITY_PER_REG) = value;
}


void gicr_sgi_config(uint32_t irq, uint32_t cfg)
{
    uint32_t offset, value;

    offset = (irq % GIC_GICR_ICFGR_PER_REG) * GIC_GICR_ICFGR_BITS_PER_REG;
    value = *REG_GIC_GICR_ICFGR0;
    value &= ~((uint32_t)0x3 << offset);
    value |= (cfg << offset);
    *REG_GIC_GICR_ICFGR0 = value;
}

void gicr_ppi_config(uint32_t irq, uint32_t cfg)
{
    uint32_t offset, value;

    offset = ((irq - GIC_SGI_MAX)% GIC_GICR_ICFGR_PER_REG) * GIC_GICR_ICFGR_BITS_PER_REG;
    value = *REG_GIC_GICR_ICFGR1;
    value &= ~((uint32_t)0x3 << offset);
    value |= (cfg << offset);
    *REG_GIC_GICR_ICFGR1 = value;
}

void gicr_clear_pending(uint32_t irq)
{
    *REG_GIC_GICR_ICPENDR0 |= 1 << (irq % GIC_GICR_ICPENDR_PER_REG);
}

void gicr_enable_irq(uint32_t irq)
{
    *REG_GIC_GICR_ISENABLER0 |= 1 << (irq % GIC_GICR_ISENABLER_PER_REG);
}

void gicr_disable_irq(uint32_t irq)
{
    *REG_GIC_GICR_ICENABLER0 |= 1 << (irq % GIC_GICR_ICENABLER_PER_REG);
}

/*Return 1 means a pending irq is found, otherwise 0*/
/*TODO: now iterate from irq 0 to max, see how to improve*/
static int gicd_find_pending_irq(uint32_t *irq)
{
    uint32_t i;
    
    uart_puts("\ngicd pending irq ");
    
    for(i = 0; i < GIC_INT_MAX; i++)
    {
        if(*REG_GIC_GICD_ISPENDR(i / GIC_GICD_ISPENDR_PER_REG) & (1 << (i % GIC_GICD_ISPENDR_PER_REG)))
        {
            *irq = i;
            uart_puthex(i);
            uart_puts(" found\n");
            return 1;
        }
    }
    
    uart_puts("not found\n");
    return 0;
}

static int gicr_find_pending_irq(uint32_t *irq)
{
    uint32_t i;
    uint32_t ispendr0 = *REG_GIC_GICR_ISPENDR0;
    
    uart_puts("\ngicr pending irq ");
    
    for(i = 0; i < GIC_PPI_MAX; i++)
    {
        if(ispendr0 & (1 << (i % GIC_GICR_ISPENDR_PER_REG)))
        {
            *irq = i;
            uart_puthex(i);
            uart_puts(" found\n");
            return 1;
        }
    }
    
    uart_puts("not found\n");
    return 0;
}

void gic_init(void)
{
    gicd_init();
    gicr_init();
    gicc_init();
}

void gic_handle(exception_t *excp __attribute__((unused)))
{
    uint32_t irq, ack;

    ack = gicc_iar();
    irq = ack & GICC_IAR_ID_MSK;

    if (irq < GIC_INT_MAX) {
		gicr_disable_irq(irq);
		gicr_clear_pending(irq);
		//at this moment, only timer irq.
		timer_handler();
		gicr_enable_irq(irq);
    }

    gicc_eoir(ack);
    gicc_dir(ack);
}
