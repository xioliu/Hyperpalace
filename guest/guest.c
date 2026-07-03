#include <stdint.h>
#include "uart.h"
#include "timer.h"
#include "ivc.h"
#include "string.h"

uint64_t guest_stack[4096] __attribute__((aligned(16)));

extern void vectors(void);

void sync_handler_c(void) {
    uart_puts("Sync exception!\n");
    while (1);
}

void irq_handler_c(void) {
    //uint32_t irq;
    //__asm__ volatile("mrs %0, icc_iar1_el1" : "=r"(irq));
    // 打印接收到的中断号
    //uart_puts("IRQ received: ");
    //uart_puthex(irq);
    //uart_puts("\n");
    
    //if (irq == 27) {
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
    //__asm__ volatile("dsb sy" : : : "memory");
    //__asm__ volatile("isb" : : : "memory");

    // 调用 C 打印函数
    uart_puts("Timer IRQ\n");
    //} else if (irq == 64 || irq == 65) {
    //    uart_puts("Doorbell received!\n");
        // 读取共享内存等
    //}
}


void _guest_start_c(void) {
    uart_init();
    uart_puts("Guest C started\n");

    __asm__ volatile("msr vbar_el1, %0" :: "r"(vectors));
    __asm__ volatile("isb" : : : "memory");

    timer_init();

    // 使能 Group1 中断
    //__asm__ volatile("msr icc_igrpen1_el1, %0" :: "r"(1ULL) : "memory");
    __asm__ volatile("msr daifclr, #2" : : : "memory");

    uint64_t mpidr;
    __asm__ volatile("mrs %0, mpidr_el1" : "=r"(mpidr));
    uint32_t cpu_id = mpidr & 0xFF;
    //uart_puts("CPU ID: ");
    //uart_puthex(mpidr & 0xFF);
    //uart_puts("\n");

    if (cpu_id == 0) {
        uart_puts("VM0: I will send doorbells\n");
    } else {
        uart_puts("VM1: I will receive doorbells\n");
    }

    while (1) {
        if (cpu_id == 0) {
            // 每隔约2秒发送一次消息
            for (volatile int i = 0; i < 2000000; i++);
            const char *msg = "Hello from VM0!";
            ivc_send(1, 0, msg, strlen(msg) + 1);
        } else {
            // 轮询共享内存标志
            uint8_t *shm = (uint8_t *)IVC_SHM_BASE;
            if (shm[0] != 0) {
                uart_puts("Doorbell received: ");
                uart_puts((const char *)(shm + 1));
                uart_puts("\n");
                shm[0] = 0;  // 清除标志
            }
            }
    }
}