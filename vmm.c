#include <stdint.h>
#include "board-qemu.h"
#include "util.h"
#include "exceptions.h"
#include "sysregs.h"
#include "gicv3.h"
#include "vmm.h"

void vmm_init(void)
{
	uint64_t hcr_el2 = HCR_VM_BIT | HCR_RW_BIT | HCR_IMO_BIT | HCR_FMO_BIT | HCR_TSC_BIT;
	sysreg_hcr_el2_write(hcr_el2);
}
