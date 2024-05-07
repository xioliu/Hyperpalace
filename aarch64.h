#ifndef __AARCH64_H
#define __AARCH64_H

/*DAIF bits*/
#define DAIF_DBG_BIT    (1<<3)
#define DAIF_ABT_BIT    (1<<2)
#define DAIF_IRQ_BIT    (1<<1)
#define DAIF_FIQ_BIT    (1<<0)

#define CNTV_CTL_ENABLE (1<<0)

#define wfi()           asm volatile("wfi" : : : "memory")

uint32_t raw_read_daif(void);
void raw_write_daif(uint32_t daif);
void disable_irq(void);
void enable_irq(void);
uint32_t raw_read_cntv_ctl(void);
void disable_cntv(void);
void enable_cntv(void);
uint32_t raw_read_cntfrq_el0(void);
uint64_t raw_read_cntvct_el0(void);
void raw_write_cntval_el0(uint64_t cntval_el0);

#endif