#include <stdint.h>
#include "init.h"
#include "aarch64.h"
#include "uart.h"

int main(void)
{
    uart_puts("main\n");
    init();
    while(1)
        wfi();
}