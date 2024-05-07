#ifndef __UART_H
#define __UART_H

#define UART_FR_CTS     (1 << 0)
#define UART_FR_DSR     (1 << 1)
#define UART_FR_DCD     (1 << 2)
#define UART_FR_BUSY    (1 << 3)
#define UART_FR_RXFE    (1 << 4)
#define UART_FR_TXFF    (1 << 5)
#define UART_FR_RXFF    (1 << 6)
#define UART_FR_TXFE    (1 << 7)
#define UART_FR_RI      (1 << 8)

void uart_putc(const char c);
void uart_puts(const char *s);

#endif