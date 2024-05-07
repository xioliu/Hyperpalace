#include <stdint.h>
#include "exceptions.h"
#include "gicv3.h"
#include "timer.h"

void init(void)
{
    gic_init();
    timer_init();
}