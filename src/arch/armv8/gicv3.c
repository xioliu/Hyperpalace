#include "gicv3.h"
#include "platform.h"
#include "sysregs.h"
#include "vgic.h"
#include "uart.h"

/* ========== 寄存器偏移与常量 ========== */
#define GICD_CTLR            0x0000
#define GICD_TYPER           0x0004
#define GICD_IIDR            0x0008
#define GICD_IGROUPR         0x0080
#define GICD_ISENABLER       0x0100
#define GICD_ICENABLER       0x0180
#define GICD_ISPENDR         0x0200
#define GICD_ICPENDR         0x0280
#define GICD_ISACTIVER       0x0300
#define GICD_ICACTIVER       0x0380
#define GICD_IPRIORITYR      0x0400
#define GICD_ITARGETSR       0x0800   /* 仅 GICv2 兼容模式，GICv3 用 IROUTER */
#define GICD_IROUTER         0x6000
#define GICD_PIDR2           0xFFE8

/* Redistributor 寄存器 (每个 CPU 一份) */
#define GICR_WAKER           0x0014
#define GICR_IGROUPR0        0x0080
#define GICR_ISENABLER0      0x0100
#define GICR_ICENABLER0      0x0180
#define GICR_IPRIORITYR      0x0400   /* SGI 优先级 */

/* CPU 接口寄存器 */
#define GICC_CTLR            0x0000
#define GICC_PMR             0x0004
#define GICC_BPR             0x0008
#define GICC_IAR             0x000C
#define GICC_EOIR            0x0010
#define GICC_RPR             0x0014
#define GICC_HPPIR           0x0018
#define GICC_DIR             0x1000   /* 偏移较大，注意 */

/* 虚拟 CPU 接口寄存器 (用于维护中断等) */
#define ICH_HCR_EL2          S3_4_C12_C11_0
#define ICH_MISR_EL2         S3_4_C12_C11_1
#define ICH_VMCR_EL2         S3_4_C12_C11_7

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

/* ========== 内部辅助：MMIO 访问 ========== */
static inline uint32_t mmio_read32(uint64_t addr)
{
    volatile uint32_t *reg = (volatile uint32_t *)addr;
    return *reg;
}

static inline void mmio_write32(uint64_t addr, uint32_t val)
{
    volatile uint32_t *reg = (volatile uint32_t *)addr;
    *reg = val;
}

static uint64_t g_gicd_base = 0ULL;
static uint64_t g_gicr_base = 0ULL;
static uint64_t g_gicc_base = 0ULL;

/* ========== 全局初始化 ========== */
void gicv3_init(void)
{
    /* 使能 GICv3 系统寄存器接口 */
    uint64_t val;
    __asm__ volatile("mrs %0, S3_0_C12_C12_5" : "=r"(val));  // ICC_SRE_EL2
    val |= (1 << 0);   // SRE bit: Enable System Register access
    __asm__ volatile("msr S3_0_C12_C12_5, %0" : : "r"(val));
    __asm__ volatile("isb");

    /* 从平台头获取基地址 */
    g_gicd_base = GICD_BASE;
    g_gicr_base = GICR_BASE;
    g_gicc_base = GICC_BASE;

    uint32_t typer = mmio_read32(g_gicd_base + GICD_TYPER);
    uint32_t lines = (typer & 0x1F) + 1U;   /* ITLinesNumber */
    uint32_t max_spi = 32U * lines - 1U;

    /* 关闭 Distributor 以便配置 */
    mmio_write32(g_gicd_base + GICD_CTLR, 0U);

    /* 设置所有中断为 Group1 NS (除 SGI/PPI 由 gicv3_init_cpu 设置) */
    for (uint32_t i = 1U; i <= (max_spi / 32U); i++) {
        mmio_write32(g_gicd_base + GICD_IGROUPR + i * 4U, 0xFFFFFFFF);
    }

    /* 将所有 SPI 路由到 CPU0 (默认) 或配置 IROUTER */
    for (uint32_t irq = GIC_SPI_BASE; irq <= max_spi; irq++) {
        /* IROUTER 每个 64 位，偏移 = GICD_IROUTER + irq*8 */
        uint64_t router_addr = g_gicd_base + GICD_IROUTER + (uint64_t)irq * 8U;
        volatile uint64_t *router = (volatile uint64_t *)router_addr;
        /* 设置亲和性为 CPU0，并使能路由 */
        *router = (0U << 0);   /* 中断路由模式，affinity0=0 */
    }

    /* 使能 Distributor (Group1 总使能) */
    mmio_write32(g_gicd_base + GICD_CTLR, 0x1U);

    /* 初始化当前 CPU 的 CPU 接口和 Redistributor */
    gicv3_init_cpu();
}

void gicv3_init_cpu(void)
{
    /* Redistributor 初始化 */
    /* 唤醒 Redistributor (GICR_WAKER.ProcessorSleep = 0) */
    //uint32_t waker = mmio_read32(g_gicr_base + GICR_WAKER);
    //waker &= ~0x1U;
    //mmio_write32(g_gicr_base + GICR_WAKER, waker);
    /* 数据同步屏障，无需轮询（QEMU 模型与部分硬件不需要等待） */
    //__asm__ volatile("dsb sy" ::: "memory");

    /* 设置 SGI 和 PPI 为 Group1 NS */
    //mmio_write32(g_gicr_base + GICR_IGROUPR0, 0xFFFFFFFF);
    /* 使能所有 SGI/PPI */
    //mmio_write32(g_gicr_base + GICR_ISENABLER0, 0xFFFFFFFF);

    /* CPU 接口初始化 */
    /* 使能 CPU 接口 (Group0 和 Group1) */
    //mmio_write32(g_gicc_base + GICC_CTLR, 0x3U);
    /* 设置最低优先级掩码 (所有中断均可通过) */
    //mmio_write32(g_gicc_base + GICC_PMR, 0xFFU);
    /* 设置二进制点 (无优先级分组) */
    //mmio_write32(g_gicc_base + GICC_BPR, 0x0U);

    /* 直接使能 CPU 接口（系统寄存器） */
    icc_write_icc_ctlr(0x1);      /* 使能 Group1 */
    icc_write_icc_pmr(0xFF);      /* 最低优先级掩码 */
    icc_write_icc_bpr1(0x0);      /* 二进制点 */

    /* 虚拟接口使能（需要在 EL2 完成） */
    uint64_t ich_hcr;
    __asm__ volatile("mrs %0, S3_4_C12_C11_0" : "=r"(ich_hcr));
    ich_hcr |= (1U << 0);  /* En */
    ich_hcr |= (1U << 1);  /* UIEn */
    ich_hcr |= (1U << 2);  /* EOICount */
    __asm__ volatile("msr S3_4_C12_C11_0, %0" : : "r"(ich_hcr));
}

/* ========== 中断应答与结束 ========== */
uint32_t gicv3_read_iar(void)
{
    return mmio_read32(g_gicc_base + GICC_IAR);
}

void gicv3_write_eoir(uint32_t iar)
{
    mmio_write32(g_gicc_base + GICC_EOIR, iar);
}

void gicv3_write_dir(uint32_t irq_id)
{
    mmio_write32(g_gicc_base + GICC_DIR, irq_id);
}

/* ========== 优先级与路由 ========== */
void gicv3_set_pmr(uint8_t pmr)
{
    mmio_write32(g_gicc_base + GICC_PMR, (uint32_t)pmr);
}

void gicv3_set_irq_target(uint32_t irq_id, uint8_t cpu_id)
{
    if (irq_id < GIC_SPI_BASE || irq_id > GIC_MAX_SPI) {
        return;
    }
    uint64_t router_addr = g_gicd_base + GICD_IROUTER + (uint64_t)irq_id * 8U;
    volatile uint64_t *router = (volatile uint64_t *)router_addr;
    *router = ((uint64_t)cpu_id & 0xFFU);
}

void gicv3_enable_irq(uint32_t irq_id, bool enable)
{
    if (irq_id > GIC_MAX_SPI) return;
    uint32_t reg_index = irq_id / 32U;
    uint32_t bit = irq_id % 32U;
    if (enable) {
        mmio_write32(g_gicd_base + GICD_ISENABLER + reg_index * 4U, (1U << bit));
    } else {
        mmio_write32(g_gicd_base + GICD_ICENABLER + reg_index * 4U, (1U << bit));
    }
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

uint8_t gicv3_get_physical_priority(uint32_t irq_id)
{
    if (irq_id > GIC_MAX_SPI) {
        return 0xFFU;   /* 最低优先级 */
    }
    uint32_t reg_index = irq_id / 4U;
    uint32_t byte_shift = (irq_id % 4U) * 8U;
    volatile uint32_t *prio_reg = (volatile uint32_t *)(g_gicd_base + 0x0400U + reg_index * 4U);
    uint32_t prio_val = *prio_reg;
    return (uint8_t)((prio_val >> byte_shift) & 0xFFU);
}
