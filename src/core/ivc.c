#include "hp_types.h"
#include "ivc.h"
#include "arch_ops.h"
#include "vm.h"
#include "uart.h"
#include "string.h"
#include "errno.h"


int32_t hp_ivc_send(uint32_t src, uint32_t dst, uint32_t doorbell) {
    // 直接向目标 VM 注入中断，中断号 = 64 + doorbell
    uint32_t irq_id = 64 + doorbell;
    if (g_arch_ops && g_arch_ops->irq_inject) {
        g_arch_ops->irq_inject(irq_id, 0x80);
        uart_puts("IVC: doorbell ");
        uart_puthex(doorbell);
        uart_puts(" from VM");
        uart_puthex(src);
        uart_puts(" to VM");
        uart_puthex(dst);
        uart_puts("\n");
        return HP_SUCCESS;
    }
    return HP_ENOSYS;
}