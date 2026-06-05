#ifndef UART_H
#define UART_H

#include <stdint.h>

/* QEMU virt 平台 UART0 地址 (PL011) */
#define UART0_BASE  0x09000000ULL
#define UART0_SIZE  0x1000ULL

/* PL011 寄存器偏移 */
#define UARTDR      0x000
#define UARTFR      0x018
#define UARTFR_TXFF (1U << 5)
#define UARTFR_RXFE (1U << 4)
#define UARTIBRD    0x024
#define UARTFBRD    0x028
#define UARTLCR_H   0x02C
#define UARTCR      0x030
#define UARTCR_UARTEN (1U << 0)
#define UARTCR_TXE   (1U << 8)
#define UARTCR_RXE   (1U << 9)

void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);
void uart_puthex(uint64_t val);

#endif /* UART_H */