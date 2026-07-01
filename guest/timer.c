#include "timer.h"

void timer_init(void) {
    uint64_t now, cval;
    __asm__ volatile("mrs %0, cntvct_el0" : "=r"(now));
    cval = now + TIMER_FREQ;
    __asm__ volatile("msr cntv_cval_el0, %0" :: "r"(cval));
    __asm__ volatile("msr cntv_ctl_el0, %0" :: "r"(1ULL) : "memory");
}