#include <stdint.h>
#include "aarch64.h"

uint32_t raw_read_current_el(void)
{
    uint32_t current_el;

    __asm__ __volatile__("mrs %0, CurrentEL\n\t" : "=r" (current_el) : : "memory");
    
    return current_el;
}

uint32_t get_current_el(void)
{
    uint32_t current_el = raw_read_current_el();
    return (current_el >> 2) & 0x03; //bits 2-3
}

uint32_t raw_read_daif(void)
{
    uint32_t daif;

    __asm__ __volatile__("mrs %0, DAIF\n\t" : "=r" (daif) : : "memory");
    return daif;
}

void raw_write_daif(uint32_t daif)
{
    __asm__ __volatile__("msr DAIF, %0\n\t" : : "r" (daif) : "memory");
}

void disable_irq(void)
{
    __asm__ __volatile("msr DAIFSet, %0\n\t" : : "i" (DAIF_IRQ_BIT) : "memory");
}

void enable_irq(void)
{
    __asm__ __volatile("msr DAIFClr, %0\n\t" : : "i" (DAIF_IRQ_BIT) : "memory");
}

uint32_t raw_read_cntv_ctl(void)
{
    uint32_t cntv_ctl;

    __asm__ __volatile__("mrs %0, CNTV_CTL_EL0\n\t" : "=r" (cntv_ctl) : : "memory");
    return cntv_ctl;
}

void disable_cntv(void)
{
    uint32_t cntv_ctl;

    cntv_ctl = raw_read_cntv_ctl();
    cntv_ctl &= ~CNTV_CTL_ENABLE;
    __asm__ __volatile__("msr CNTV_CTL_EL0, %0\n\t" : : "r" (cntv_ctl) : "memory");
}

void enable_cntv(void)
{
    uint32_t cntv_ctl;

    cntv_ctl = raw_read_cntv_ctl();
    cntv_ctl |= CNTV_CTL_ENABLE;
    __asm__ __volatile__("msr CNTV_CTL_EL0, %0\n\t" : : "r" (cntv_ctl) : "memory");
}

uint32_t raw_read_cntfrq_el0(void)
{
    uint32_t cntfrq_el0;

    __asm__ __volatile__("mrs %0, CNTFRQ_EL0\n\t" : "=r" (cntfrq_el0) : : "memory");
    return cntfrq_el0;
}

uint64_t raw_read_cntvct_el0(void)
{
    uint64_t cntvct_el0;

    __asm__ __volatile__("mrs %0, CNTVCT_EL0\n\t" : "=r" (cntvct_el0) : : "memory");
    return cntvct_el0;
}

void raw_write_cntval_el0(uint64_t cntval_el0)
{
    __asm__ __volatile__("msr CNTV_CVAL_EL0, %0\n\t" : : "r" (cntval_el0) : "memory");
}