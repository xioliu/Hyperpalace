#include "platform.h"
#include "gicv3.h"
#include "sysregs.h"
#include "vgic.h"
#include "uart.h"

/* CPU 接口寄存器 */
#define GICC_CTLR            0x0000
#define GICC_PMR             0x0004
#define GICC_BPR             0x0008
#define GICC_IAR             0x000C
#define GICC_EOIR            0x0010
#define GICC_RPR             0x0014
#define GICC_HPPIR           0x0018
#define GICC_DIR             0x1000   /* 偏移较大，注意 */

/* 维护中断标识 */
#define ICH_MISR_EOI         (1U << 0)
#define ICH_MISR_U           (1U << 1)
#define ICH_MISR_LREN        (1U << 2)
#define ICH_MISR_NP          (1U << 3)
#define ICH_MISR_VGRP0E      (1U << 6)
#define ICH_MISR_VGRP0D      (1U << 7)
#define ICH_MISR_VGRP1E      (1U << 8)
#define ICH_MISR_VGRP1D      (1U << 9)

/* SPI 起始中断号 */
#define GIC_SPI_BASE         32U
#define GIC_MAX_SPI          1019U

/* ========== 全局初始化 ========== */
void gicv3_init(void)
{
    uint64_t val;

    mmio_write32(GICD_BASE + GICD_CTLR, 0);

    //mmio_write32(GICD_BASE + GICD_IGROUPR(0), 0xFFFFFFFF);
    //mmio_write32(GICD_BASE + GICD_ICENABLER(0), 0xFFFFFFFF);
    //mmio_write32(GICD_BASE + GICD_ICPENDR(0), 0xFFFFFFFF);
    //mmio_write32(GICD_BASE + GICD_ICACTIVER(0), 0xFFFFFFFF);
    //mmio_write32(GICD_BASE + GICD_IPRIORITYR(0), 0xFFFFFFFF);

    // 使能GICD， ARE_NS=1 bit [4] 打开亲和路由， 否则GICD 处于 “旧版 GICv2 兼容模式”
    mmio_write32(GICD_BASE + GICD_CTLR, 0x12);
    // 等待GICD启动完成
    while ((mmio_read32(GICD_BASE + GICD_CTLR) & 0x12) != 0x12);

    /* 使能 GICv3 系统寄存器接口，GICv3在SRE=0时，很多关键 MMIO 寄存器是 RAZ/WI */
    /* 目的：强制你先开 SRE，再用系统寄存器接口（ICC_*_EL2）** 来配置 GIC */
    val = read_icc_sre_el2();
    val |= 0x1; // SRE=1 启用系统寄存器
    write_icc_sre_el2(val);
    isb();

    // 唤醒GICR
    //mmio_write32(GICR_BASE + GICR_WAKER, 0x0);
    //while (mmio_read32(GICR_BASE + GICR_WAKER) & (0x1 << 2));
    
    mmio_write32(GICR_BASE + GICR_IGROUPR0, 0xFFFFFFFF);
    mmio_write32(GICR_BASE + GICR_ISENABLER0, 0xFFFFFFFF);   // 使能所有 SGI/PPI，包括 26
    //mmio_write32(GICR_BASE + GICR_ICENABLER0, 0xFFFFFFFF);
    //mmio_write32(GICR_BASE + GICR_ICPENDR, 0xFFFFFFFF);
    //mmio_write32(GICR_BASE + GICR_ICACTIVER0, 0xFFFFFFFF);
    //mmio_write32(GICR_BASE + GICR_IPRIORITYR0, 0xFFFFFFFF);
    // 使能GICR
    mmio_write32(GICR_BASE + GICR_CTLR, 0x1);

    write_icc_pmr_el1(0xFF);          // 优先级掩码全开
    write_icc_ctlr_el1(0x1);          // EOImode=1
    write_icc_igrpen1_el1(0x1);       // 使能非安全Group1中断

    gicv3_init_cpu();
}

void gicv3_init_cpu(void)
{
    uint64_t val;
    uint32_t i, max_lr;

    // 先清零ICH_HCR_EL2（禁用所有虚拟化功能）
    write_ich_hcr_el2(0);
    isb();
    // 配置ICH_HCR_EL2（启用虚拟中断注入）
    val = 0;
    val |= (1 << 0);  // En=1: 全局启用虚拟中断
    val |= (1 << 1);  // UIE=1: 启用维护中断（LR空了通知Hypervisor）
    val |= (1 << 2);  // LRENPIE=1
    write_ich_hcr_el2(val);
    isb();

    // 获取LR寄存器数量（QEMU 7.2.0是8个）
    max_lr = (read_ich_vtr_el2() & 0xF) + 1;

    // 清空所有LR寄存器（避免残留旧中断）
    for (i = 0; i < max_lr; i++) {
        write_ich_lr_el2(i, 0);
    }

    // 配置ICH_VMCR_EL2（Guest中断全局配置）
    val = 0;
    val |= (0xFF << 24); // VPMR=0xFF: Guest不屏蔽任何优先级
    val |= (0x0 << 9);   // VEOIM=0: Guest 写 EOIR 自动取消激活
    val |= (0x1 << 3);   // VFIQEn=1: 允许 Guest 接收 FIQ
    val |= (0x1 << 1);   // VENG1=1: 只使能虚拟 Group1 中断
    // val |= (0x1 << 0); // 删除这行！关闭虚拟 Group0 中断
    write_ich_vmcr_el2(val);
    isb();
}

void gicv3_set_irq_target(uint32_t irq_id, uint8_t cpu_id)
{
    if (irq_id < GIC_SPI_BASE || irq_id > GIC_MAX_SPI) {
        return;
    }
    uint64_t router_addr = GICD_BASE + GICD_IROUTER + (uint64_t)irq_id * 8U;
    volatile uint64_t *router = (volatile uint64_t *)router_addr;
    *router = ((uint64_t)cpu_id & 0xFFU);
}

void gicv3_enable_irq(uint32_t irq_id, bool enable)
{
    if (irq_id > GIC_MAX_SPI) return;
    uint32_t reg_index = irq_id / 32U;
    uint32_t bit = irq_id % 32U;
    if (enable) {
        mmio_write32(GICD_BASE + GICD_ISENABLER(reg_index), (1U << bit));
    } else {
        mmio_write32(GICD_BASE + GICD_ICENABLER(reg_index), (1U << bit));
    }
}

/* ========== 辅助函数 ========== */
bool gicv3_irq_belongs_to_vm(uint32_t irq_id)
{
    (void)irq_id;
    /* 静态分区下，所有物理中断直通给当前运行的 VM */
    return true;
}

/* ========== 中断应答与结束 ========== */
uint32_t gicv3_read_iar(void)
{
    return mmio_read32(GICC_BASE + GICC_IAR);
}

void gicv3_write_eoir(uint32_t iar)
{
    mmio_write32(GICC_BASE + GICC_EOIR, iar);
}

void gicv3_write_dir(uint32_t irq_id)
{
    mmio_write32(GICC_BASE + GICC_DIR, irq_id);
}

/* Hypervisor 自身中断处理（当前仅示例定时器） */
void gicv3_handle_irq(void)
{
    uart_puts("gicv3_handle_irq\n");
    uint32_t iar = gicv3_read_iar();
    uint32_t irqid = iar & 0x3FFU;
    if (irqid >= 1020U) return;

    if (irqid == 26U) {   /* EL2 物理定时器 */
        /* 处理定时器中断 */
        __asm__ volatile("mrs x0, cntp_ctl_el0" ::: "x0");
        uart_puts("EL2 timer IRQ\n");
    }

    gicv3_write_eoir(iar);
    gicv3_write_dir(irqid);
}

void gicv3_maintenance_handler(void)
{
    uint32_t misr;
    __asm__ volatile("mrs %0, S3_4_C12_C11_1" : "=r"(misr));
    armv8_vgic_handle_maintenance(misr);
}
#if 0
static inline void gicv3_maintenance_irq_handler(void) {
    uint64_t misr, lr_val;
    unsigned int i, max_lr;

    // 读取维护中断状态
    misr = read_ich_misr_el2();

    // 处理EOI中断（Guest已经处理完中断）
    if (misr & (1 << 0)) { // EOI位
        max_lr = (read_ich_vtr_el2() & 0xF) + 1;
        for (i = 0; i < max_lr; i++) {
            lr_val = read_ich_lr_el2(i);
            // VEOIM=0时，Guest写EOIR后LR状态自动变为Inactive(2)
            if ((lr_val >> 62) == 2) {
                write_ich_lr_el2(i, 0); // 清空LR，释放资源
            }
        }
    }

    // 处理LR空中断（UIE触发）
    if (misr & (1 << 1)) { // U位
        // 所有LR都空了，可以在这里注入更多挂起的虚拟中断
    }

    // 处理无挂起中断（NPIE触发，如果你开了的话）
    if (misr & (1 << 2)) { // NP位
        // 没有新的挂起中断
    }
}
#endif
uint8_t gicv3_get_physical_priority(uint32_t irq_id)
{
    if (irq_id > GIC_MAX_SPI) {
        return 0xFFU;   /* 最低优先级 */
    }
    uint32_t reg_index = irq_id / 4U;
    uint32_t byte_shift = (irq_id % 4U) * 8U;
    volatile uint32_t *prio_reg = (volatile uint32_t *)(GICD_BASE + 0x0400U + reg_index * 4U);
    uint32_t prio_val = *prio_reg;
    return (uint8_t)((prio_val >> byte_shift) & 0xFFU);
}
