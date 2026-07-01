#include "hp_types.h"
#include "platform.h"
#include "gicv3.h"
#include "armv8_vm.h"
#include "sysregs.h"
#include "vgic.h"
#include "vtimer.h"
#include "arch_ops.h"
#include "uart.h"
#include "util.h"

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

void gicr_init(uint32_t cpuid)
{
    uint32_t i, nr;
    uart_puts("gicr_init\n");

    if(cpuid == 0) {
        *REG_GIC_GICR_WAKER &= ~GICR_WAKER_ProcessorSleep_BIT;
        while(*REG_GIC_GICR_WAKER & GICR_WAKER_ChildrenASleep_BIT);

        nr = NUMBER(GIC_INT_MAX, GIC_GICR_INT_PER_REG);

        *REG_GIC_GICR_IGROUPR0 = ~0U;//0x080b0080
        *REG_GIC_GICR_ICENABLER0 = ~0U;//0x080b0180
        *REG_GIC_GICR_ICPENDR0 = ~0U;//0x080b0280
        *REG_GIC_GICR_ICACTIVER0 = ~0U;//0x080b0380

        for(i = 0; i < nr; i++)
            *REG_GIC_GICR_IPRIORITYR(i) = ~0U;
    } 
    else if(cpuid == 1) {
        *(volatile uint32_t *)(uintptr_t)(0x080c0014ULL) &= ~GICR_WAKER_ProcessorSleep_BIT;
        while(*(volatile uint32_t *)(uintptr_t)(0x080c0014ULL) & GICR_WAKER_ChildrenASleep_BIT);

        nr = NUMBER(GIC_INT_MAX, GIC_GICR_INT_PER_REG);
        *(volatile uint32_t *)(uintptr_t)(0x080d0080ULL) = ~0U;
        *(volatile uint32_t *)(uintptr_t)(0x080d0180ULL) = ~0U;
        *(volatile uint32_t *)(uintptr_t)(0x080d0280ULL) = ~0U;
        *(volatile uint32_t *)(uintptr_t)(0x080d0380ULL) = ~0U;

        for(i = 0; i < nr; i++)
            *(volatile uint32_t *)(uintptr_t)(0x080d0400ULL + i * 4) = ~0U;
    }
} 



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
    mmio_write32(GICD_BASE + GICD_CTLR, 0x22);
    // 等待GICD启动完成
    while ((mmio_read32(GICD_BASE + GICD_CTLR) & 0x12) != 0x12);

    /* 使能 GICv3 系统寄存器接口，GICv3在SRE=0时，很多关键 MMIO 寄存器是 RAZ/WI */
    /* 目的：强制你先开 SRE，再用系统寄存器接口（ICC_*_EL2）** 来配置 GIC */
    val = read_icc_sre_el2();
    val |= 0x9; // SRE=1 启用系统寄存器
    write_icc_sre_el2(val);
    isb();

    icc_write_icc_pmr(0xFF);      /* 最低优先级掩码 */
    icc_write_icc_bpr1(0x0);      /* 二进制点 */
    icc_write_icc_ctlr(0x2);      /* 使能 Group1 */
    sysreg_ich_hcr_el2_write(sysreg_ich_hcr_el2_read() | ICH_HCR_LRENPIE_BIT);
    sysreg_icc_igrpen1_el1_write(0x1);

    gicv3_init_cpu();
}

void gicv3_init_cpu(void)
{
    uint64_t val;
    uint32_t i, max_lr;
    //uart_puts("gicv3_init_cpu ");
    uint32_t cpu_id = hp_arch_get_current_cpu_id();
    //uart_puthex(cpu_id);
    //uart_puts("\n");

    /* 初始化当前 CPU 的 Redistributor */
    gicr_init(cpu_id);

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

    // 获取LR寄存器数量（QEMU 7.2.0是4个）
    max_lr = (read_ich_vtr_el2() & 0xF) + 1;
    //uart_puts("max num of lr is ");
    //uart_puthex(max_lr);
    //uart_puts("\n");

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
    uint64_t router_addr = GICD_BASE + GICD_IROUTER(0) + (uint64_t)irq_id * 8U;
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

void gicr_set_priority(uint32_t irq, uint32_t pri)
{
    uint32_t offset, value;

    offset = (irq % GIC_GICR_INTPRIORITY_PER_REG) * GIC_GICR_INTPRIORITY_SIZE_PER_REG;
    value = *REG_GIC_GICR_IPRIORITYR(irq / GIC_GICR_INTPRIORITY_PER_REG);
    value &= ~((uint32_t)0xff << offset);
    value |= (pri << offset);
    *REG_GIC_GICR_IPRIORITYR(irq / GIC_GICR_INTPRIORITY_PER_REG) = value;
}

void gicr_set_priority_cpu1(uint32_t irq, uint32_t pri)
{
    uint32_t offset, value;

    offset = (irq % GIC_GICR_INTPRIORITY_PER_REG) * GIC_GICR_INTPRIORITY_SIZE_PER_REG;
    value = *(volatile uint32_t *)0x080d0418;
    value &= ~((uint32_t)0xff << offset);
    value |= (pri << offset);
    *(volatile uint32_t *)0x080d0418 = value;
}

void gicr_sgi_config(uint32_t irq, uint32_t cfg)
{
    uint32_t offset, value;

    offset = (irq % GIC_GICR_ICFGR_PER_REG) * GIC_GICR_ICFGR_BITS_PER_REG;
    value = *REG_GIC_GICR_ICFGR0;
    value &= ~((uint32_t)0x3 << offset);
    value |= (cfg << offset);
    *REG_GIC_GICR_ICFGR0 = value;
}

void gicr_ppi_config(uint32_t irq, uint32_t cfg)
{
    uint32_t offset, value;

    offset = ((irq - GIC_SGI_MAX)% GIC_GICR_ICFGR_PER_REG) * GIC_GICR_ICFGR_BITS_PER_REG;
    value = *REG_GIC_GICR_ICFGR1;
    value &= ~((uint32_t)0x3 << offset);
    value |= (cfg << offset);
    *REG_GIC_GICR_ICFGR1 = value;
}

void gicr_ppi_config_cpu1(uint32_t irq, uint32_t cfg)
{
    uint32_t offset, value;

    offset = ((irq - GIC_SGI_MAX)% GIC_GICR_ICFGR_PER_REG) * GIC_GICR_ICFGR_BITS_PER_REG;
    value = *(volatile uint32_t *)0x080d0c04;
    value &= ~((uint32_t)0x3 << offset);
    value |= (cfg << offset);
    *(volatile uint32_t *)0x080d0c04 = value;
}

void gicr_clear_pending(uint32_t irq)
{
    *REG_GIC_GICR_ICPENDR0 |= 1 << (irq % GIC_GICR_ICPENDR_PER_REG);
}

void gicr_clear_pending_cpu1(uint32_t irq)
{
    *(volatile uint32_t *)0x080d0280 |= 1 << (irq % GIC_GICR_ICPENDR_PER_REG);
}

void gicr_enable_irq(uint32_t irq)
{
    *REG_GIC_GICR_ISENABLER0 |= 1 << (irq % GIC_GICR_ISENABLER_PER_REG);
}

void gicr_enable_irq_cpu1(uint32_t irq)
{
    *(volatile uint32_t *)0x080d0100 |= 1 << (irq % GIC_GICR_ISENABLER_PER_REG);
}

void gicr_disable_irq(uint32_t irq)
{
    *REG_GIC_GICR_ICENABLER0 |= 1 << (irq % GIC_GICR_ICENABLER_PER_REG);
}

void gicr_disable_irq_cpu1(uint32_t irq)
{
    *(volatile uint32_t *)0x080d0180 |= 1 << (irq % GIC_GICR_ICENABLER_PER_REG);
}

/* ========== 辅助函数 ========== */
bool gicv3_irq_belongs_to_vm(uint32_t irq_id)
{
    (void)irq_id;
    /* 静态分区下，所有物理中断直通给当前运行的 VM */
    return true;
}

/* Hypervisor 自身中断处理（当前仅示例定时器） */
void gicv3_handle_irq(void)
{
    uart_puts("gicv3_handle_irq\n");
    uint32_t iar = gicc_iar();
    uint32_t irqid = iar & 0x3FFU;
    uart_puthex(iar);
    uart_puts("\n");

    if (irqid >= 1020U) return;

    //gicr_disable_irq(irqid);
    //gicr_clear_pending(irqid);
    //gicr_enable_irq(irqid);

    //timer_handler();

    gicc_eoir(iar);
    gicc_dir(iar);
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

void gicv3_route_irq_to_el2(uint32_t irq_id)
{
    if (irq_id < GIC_SPI_BASE || irq_id > GIC_MAX_SPI) {
        return;  // 只处理SPI中断，SGI/PPI是CPU私有，不需要路由
    }

    uint32_t reg_index = irq_id / 32U;
    uint32_t bit = irq_id % 32U;

    /* 第一步：将中断设置为Group 1非安全 */
    uint32_t igroupr = mmio_read32(GICD_BASE + GICD_IGROUPR(reg_index));
    igroupr |= (1U << bit);  // 1 = Group 1 NS
    mmio_write32(GICD_BASE + GICD_IGROUPR(reg_index), igroupr);

    /* 第二步：获取当前CPU的亲和性 */
    uint64_t mpidr = read_mpidr_el1();
    uint64_t aff = mpidr & 0xffffffULL;  // 提取Aff0+Aff1+Aff2

    /* 第三步：设置中断路由到当前CPU */
    uint64_t irouter = aff;
    irouter &= ~GICD_IROUTER_IRM;  // IRM=0：路由到指定CPU
    mmio_write64(GICD_BASE + GICD_IROUTER(irq_id), 0);

    dsb();  // 确保寄存器写入生效
}