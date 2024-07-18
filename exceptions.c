#include <stdint.h>
#include "util.h"
#include "exceptions.h"
#include "sysregs.h"
#include "uart.h"
#include "gicv3.h"
#include "aarch64.h"

void exceptions_handler(exception_t *exc __attribute__((unused)))
{
    uart_puts("exception\n");
}

void irq_handler(exception_t *excp)
{
    //uart_puts("irq\n");
    gic_handle(excp);
}

void common_trap_handler(exception_t *exc)
{
    //uart_puts("common_trap_handler exc_type = ");
    //uart_puthex(exc->exc_type);
    //uart_puts("\n");

    /*now only timer irq*/
    if(( exc->exc_type & 0xff) == EL2_EXC_IRQ_SPX)
    {
        irq_handler(exc);
    }

}

