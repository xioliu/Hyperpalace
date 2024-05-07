#include <stdint.h>
#include "uart.h"

volatile unsigned int * const UART0DR = (unsigned int *) 0x09000000;
volatile unsigned int * const UART0FR = (unsigned int *) 0x09000018;

void uart_putc(const char c)
{
    while((*UART0FR) & UART_FR_TXFF);
    *UART0DR = c;
}

void uart_puts(const char *s)
{
    unsigned int i = 0;
    while(s[i])
        uart_putc((unsigned char)s[i++]);
}