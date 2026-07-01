#include "uart.h"

#define UART_BASE   0x09000000ULL
#define UARTDR      (UART_BASE + 0x000)
#define UARTFR      (UART_BASE + 0x018)
#define UARTFR_TXFF (1U << 5)

void uart_init(void) {
    // 由 Hypervisor 已初始化，无需操作
}

void uart_putc(char c) {
    while (*(volatile uint32_t *)UARTFR & UARTFR_TXFF);
    *(volatile uint32_t *)UARTDR = (uint32_t)c;
}

void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

void uart_puthex(uint64_t val) {
    const char hex[] = "0123456789abcdef";
    char buf[19] = "0x0000000000000000";
    for (int i = 17; i >= 2; i--) {
        buf[i] = hex[val & 0xFULL];
        val >>= 4;
    }
    uart_puts(buf);
}