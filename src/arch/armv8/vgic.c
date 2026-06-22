#include "platform.h"
#include "gicv3.h"
#include "vgic.h"
#include "sysregs.h"
#include "uart.h"

#define VGIC_LR_COUNT      16U
#define DEFAULT_PRIORITY   0x80U

#if 0
/* 读取物理中断的当前优先级（可选） */
static uint8_t read_physical_priority(uint32_t irq_id)
{
    /* GICD_IPRIORITYR 每个字节一个中断 */
    uint32_t reg_index = irq_id / 4U;
    uint32_t byte_shift = (irq_id % 4U) * 8U;
    volatile uint32_t *prio_reg = (volatile uint32_t *)(GICD_BASE + 0x0400U + reg_index * 4U);
    uint32_t prio_val = *prio_reg;
    return (uint8_t)((prio_val >> byte_shift) & 0xFFU);
}
#endif

/* 注入指定优先级的虚拟中断 */
void armv8_vgic_inject(uint32_t irq_id, uint8_t priority)
{
    for (uint32_t i = 0U; i < VGIC_LR_COUNT; i++) {
        uint64_t lr = read_ich_lr_el2(i);
        if ((lr & (0x3ULL << 62)) == 0U) {   /* State bits [63:62] == 0 (Invalid) */
            uint64_t new_lr = irq_id;                                  /* vINTID[31:0]：Guest看到的中断号 */
            new_lr |= ((irq_id & 0x1FFFULL) << 32);            /* pINTID[44:32] */
            new_lr |= (0x1ULL << 62);                          /* State: Pending */
            new_lr |= (0x1ULL << 61);                          /* HW[61] */
            new_lr |= (0x1ULL << 60);                          /* Group[60]=1 vIRQ */
            new_lr |= ((uint64_t)priority << 48);             /* priority[55:48] */
            write_ich_lr_el2(i, new_lr);
            uart_puts("armv8_vgic_inject:");
            uart_puthex(new_lr);
            uart_puts("\n");
            return;
        }
    }
    /* LR 已满，触发下溢（设置 VI） */
    uart_puts("set vi\n");
    uint64_t hcr = read_hcr_el2();
    hcr |= (1U << 7);   /* VI */
    write_hcr_el2(hcr);
}

/* 初始化 vGIC 接口 */
void armv8_vgic_init(void)
{
    write_ich_hcr_el2(1U);   /* En = 1 */
}

/* 处理虚拟化维护中断 */
void armv8_vgic_handle_maintenance(uint32_t misr)
{
    /* EOI 维护：VM 已通过虚拟 CPU 接口完成中断服务 */
    if (misr & (1U << 0)) {       /* ICH_MISR_EL2.EOI */
        /* 硬件通常已自动将对应 LR 的状态从 Active 转为 Invalid
           并可能解激活物理中断（因为 HW=1），无需额外操作。 */
    }

    /* Underflow：LR 不够容纳挂起的中断 */
    if (misr & (1U << 1)) {       /* ICH_MISR_EL2.U */
        /* 尝试清理一些已无效的 LR 或直接置 VI */
        uint64_t hcr = read_hcr_el2();
        hcr |= (1U << 7);         /* VI = 1，通知 VM 有挂起中断 */
        write_hcr_el2(hcr);
    }

    /* NP (no pending) 等其他事件可忽略 */
}

/* 保存状态（暂空） */
void armv8_vgic_save_state(void) { }

/* 恢复状态：只需确保虚拟接口使能 */
void armv8_vgic_restore_state(void)
{
    uint32_t hcr = read_ich_hcr_el2();
    if ((hcr & 1U) == 0U) {
        write_ich_hcr_el2(hcr | 1U);
    }
}