#include <stdint.h>
#include "exceptions.h"
#include "uart.h"
#include "gicv3.h"

void exceptions_handler(exception_t *exc __attribute__((unused)))
{
    uart_puts("exception\n");
}

void irq_handler(exception_t *excp)
{
    uart_puts("irq\n");
    irq_handle(excp);
}

void common_trap_handler(exception_t *exc)
{
    uart_puts("common_trap_handler\n");

    /*now only timer irq*/
    if(( exc->exc_type & 0xff) == EL2_EXC_IRQ_SPX)
    {
        irq_handler(exc);
    }

}

