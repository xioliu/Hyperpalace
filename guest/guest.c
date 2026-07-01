#include <stdint.h>
#include "uart.h"
#include "timer.h"
#include "ivc.h"

uint64_t guest_stack[4096] __attribute__((aligned(16)));

extern void vectors(void);

void sync_handler_c(void) {
    uart_puts("Sync exception!\n");
    while (1);
}

void irq_handler_c(void) {
    //uint32_t irq;
    //__asm__ volatile("mrs %0, icc_iar1_el1" : "=r"(irq));
    
    // 清除定时器
    __asm__ volatile("msr cntv_ctl_el0, xzr" : : : "memory");
    __asm__ volatile("isb" : : : "memory");

    // 重设比较值
    uint64_t now, cval;
    __asm__ volatile("mrs %0, cntvct_el0" : "=r"(now));
    cval = now + 62500000ULL;
    __asm__ volatile("msr cntv_cval_el0, %0" :: "r"(cval));
    __asm__ volatile("isb" : : : "memory");

    // 重新使能定时器
    __asm__ volatile("msr cntv_ctl_el0, %0" :: "r"(1ULL) : "memory");
    __asm__ volatile("isb" : : : "memory");

    // 真实硬件可能需要
    //__asm__ volatile("msr icc_eoir1_el1, %0" :: "r"(irq));
    //__asm__ volatile("msr icc_dir_el1, %0" :: "r"(irq));
    //__asm__ volatile("isb" : : : "memory");

    // 调用 C 打印函数
    //uart_puts("Timer IRQ\n");
}


void _guest_start_c(void) {
    uart_init();
    uart_puts("Guest C started\n");

    __asm__ volatile("msr vbar_el1, %0" :: "r"(vectors));
    __asm__ volatile("isb" : : : "memory");

    timer_init();

    __asm__ volatile("msr daifclr, #2" : : : "memory");

    uint64_t mpidr;
    __asm__ volatile("mrs %0, mpidr_el1" : "=r"(mpidr));
    uart_puts("CPU ID: ");
    uart_puthex(mpidr & 0xFF);
    uart_puts("\n");

    while (1) {
    }
}