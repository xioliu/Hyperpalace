#include <stdint.h>
#include "util.h"
#include "exceptions.h"
#include "sysregs.h"
#include "gicv3.h"
#include "timer.h"
#include "vmm.h"

void init(void)
{
    vmm_init();
    gic_init();
    timer_init();
}
