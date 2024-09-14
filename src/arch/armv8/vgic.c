#include "gicv3.h"
#include "platform.h"
#include "vgic.h"
#include "sysregs.h"

/* 寄存器访问 */
static inline uint32_t read_ich_hcr_el2(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, S3_4_C12_C11_0" : "=r"(val));
    return (uint32_t)val;
}
static inline void write_ich_hcr_el2(uint32_t val) {
    __asm__ volatile("msr S3_4_C12_C11_0, %0" : : "r"((uint64_t)val));
}

static inline uint64_t read_ich_lr_el2(uint32_t n) {
    uint64_t val = 0;
    /* 简化：假设编译器支持直接寄存器名；若不支持可展开 */
    switch (n) {
        case 0:  __asm__ volatile("mrs %0, S3_4_C12_C12_0" : "=r"(val)); break;
        case 1:  __asm__ volatile("mrs %0, S3_4_C12_C12_1" : "=r"(val)); break;
        case 2:  __asm__ volatile("mrs %0, S3_4_C12_C12_2" : "=r"(val)); break;
        case 3:  __asm__ volatile("mrs %0, S3_4_C12_C12_3" : "=r"(val)); break;
        case 4:  __asm__ volatile("mrs %0, S3_4_C12_C12_4" : "=r"(val)); break;
        case 5:  __asm__ volatile("mrs %0, S3_4_C12_C12_5" : "=r"(val)); break;
        case 6:  __asm__ volatile("mrs %0, S3_4_C12_C12_6" : "=r"(val)); break;
        case 7:  __asm__ volatile("mrs %0, S3_4_C12_C12_7" : "=r"(val)); break;
        default: break;
    }
    return val;
}
static inline void write_ich_lr_el2(uint32_t n, uint64_t val) {
    switch (n) {
        case 0:  __asm__ volatile("msr S3_4_C12_C12_0, %0" : : "r"(val)); break;
        case 1:  __asm__ volatile("msr S3_4_C12_C12_1, %0" : : "r"(val)); break;
        case 2:  __asm__ volatile("msr S3_4_C12_C12_2, %0" : : "r"(val)); break;
        case 3:  __asm__ volatile("msr S3_4_C12_C12_3, %0" : : "r"(val)); break;
        case 4:  __asm__ volatile("msr S3_4_C12_C12_4, %0" : : "r"(val)); break;
        case 5:  __asm__ volatile("msr S3_4_C12_C12_5, %0" : : "r"(val)); break;
        case 6:  __asm__ volatile("msr S3_4_C12_C12_6, %0" : : "r"(val)); break;
        case 7:  __asm__ volatile("msr S3_4_C12_C12_7, %0" : : "r"(val)); break;
        default: break;
    }
}

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
        if ((lr & 0x1U) == 0U) {   /* State bits [1:0] == 0 (Invalid) */
            uint64_t new_lr = (irq_id & 0x3FFU)
                            | (((uint64_t)priority & 0xFCU) << 32)   /* priority[7:2] */
                            | 0x1U                                    /* State: Pending */
                            | (1U << 10)                              /* HW */
                            | ((uint64_t)irq_id << 48);               /* pINTID */
            write_ich_lr_el2(i, new_lr);
            return;
        }
    }
    /* LR 已满，触发下溢（设置 VI） */
    uint64_t hcr = read_hcr_el2();
    hcr |= (1U << 3);   /* VI */
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
        hcr |= (1U << 3);         /* VI = 1，通知 VM 有挂起中断 */
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