#include "uart.h"
#include <stddef.h>

/* 辅助函数：读取/写入 UART 寄存器 */
static inline uint32_t uart_read(uint32_t offset)
{
    volatile uint32_t *reg = (volatile uint32_t *)(UART0_BASE + offset);
    return *reg;
}

static inline void uart_write(uint32_t offset, uint32_t value)
{
    volatile uint32_t *reg = (volatile uint32_t *)(UART0_BASE + offset);
    *reg = value;
}

void uart_init(void)
{
    /* 禁用 UART 以便配置 */
    uart_write(UARTCR, 0);

    /* 配置波特率：115200 @ 24MHz 时钟 */
    uart_write(UARTIBRD, 13);   /* 整数部分 */
    uart_write(UARTFBRD, 0);    /* 小数部分 */

    /* 8 数据位，无奇偶，1 停止位，使能 FIFO */
    uart_write(UARTLCR_H, (3U << 5));  /* WLEN = 8 bits */

    /* 使能 UART、发送、接收 */
    uart_write(UARTCR, UARTCR_UARTEN | UARTCR_TXE | UARTCR_RXE);
}

void uart_putc(char c)
{
    /* 等待发送 FIFO 未满 */
    while (uart_read(UARTFR) & UARTFR_TXFF) {
        /* spin */
    }
    uart_write(UARTDR, (uint32_t)c);
}

void uart_puts(const char *s)
{
    if (s == NULL) return;
    while (*s != '\0') {
        uart_putc(*s);
        s++;
    }
}

void uart_puthex(uint64_t val)
{
    static const char hex_chars[] = "0123456789abcdef";
    char buf[19];  /* "0x" + 16 hex digits + null */
    buf[0] = '0';
    buf[1] = 'x';
    for (int32_t i = 17; i >= 2; i--) {
        buf[i] = hex_chars[val & 0xFULL];
        val >>= 4;
    }
    buf[18] = '\0';
    uart_puts(buf);
}