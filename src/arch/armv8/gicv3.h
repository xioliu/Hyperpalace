#ifndef GICV3_H
#define GICV3_H

#include <stdint.h>
#include <stdbool.h>

// GICD 寄存器偏移
#define GICD_CTLR          0x000
#define GICD_TYPER         0x004
#define GICD_IGROUPR(n)    (0x080 + (n)*4)
#define GICD_ISENABLER(n)  (0x100 + (n)*4)
#define GICD_ICENABLER(n)  (0x180 + (n)*4)
#define GICD_ICPENDR(n)    (0x280 + (n)*4)
#define GICD_ICACTIVER(n)  (0x380 + (n)*4)
#define GICD_IPRIORITYR(n) (0x400 + (n)*4)
#define GICD_ITARGETSR(n)  (0x800 + (n)*4)
#define GICD_ICFGR(n)      (0xC00 + (n)*4)
#define GICD_SGIR          0xF00
#define GICD_IROUTER(n)    (0x6000 + (n)*4)

// GICR 寄存器偏移 (每个CPU 64KB)
#define GICR_CTLR          0x000
#define GICR_IIDR          0x004
#define GICR_TYPER         0x008
#define GICR_WAKER         0x014
#define GICR_IGROUPR0      0x080
#define GICR_ISENABLER0    0x100
#define GICR_ICENABLER0    0x180
#define GICR_ICPENDR       0x280

#define GICR_ICACTIVER0    0x380
#define GICR_IPRIORITYR0   0x400
#define GICR_ICFGR0        0xC00

/* GICD_CTLR 位定义 */
#define GICD_CTLR_ENABLE_G0  (1U << 0)
#define GICD_CTLR_ENABLE_G1NS (1U << 1)

/* GICD_IROUTER 位定义 */
#define GICD_IROUTER_IRM     (1ULL << 31)  // 中断路由模式：0=指定CPU，1=广播
#define GICD_IROUTER_AFF_SHIFT 0
#define GICD_IROUTER_AFF_MASK  0xffffffULL

#define GIC_GICR_BASE GICR_BASE

#define GIC_GICR_CTLR                       (GIC_GICR_BASE + 0x04)
#define GIC_GICR_IIDR                       (GIC_GICR_BASE + 0x04)
#define GIC_GICR_TYPER                      (GIC_GICR_BASE + 0x08)
#define GIC_GICR_STATUSR                    (GIC_GICR_BASE + 0x10)
#define GIC_GICR_WAKER                      (GIC_GICR_BASE + 0x14)
#define GIC_GICR_SETLPIR                    (GIC_GICR_BASE + 0x40)
#define GIC_GICR_CLRLPIR                    (GIC_GICR_BASE + 0x48)
#define GIC_GICR_PROPBASER                  (GIC_GICR_BASE + 0x70)
#define GIC_GICR_PENDBASER                  (GIC_GICR_BASE + 0x78)
#define GIC_GICR_INVLPIR                    (GIC_GICR_BASE + 0xA0)
#define GIC_GICR_INVALLR                    (GIC_GICR_BASE + 0xB0)
#define GIC_GICR_SYNCR                      (GIC_GICR_BASE + 0xC0)
#define GIC_GICRID(N)                       (GIC_GICR_BASE + 0xFFD0 + (N) * 4)

/* 注意这些GICR SGI_base寄存器的偏移 */
#define GIC_GICR_IGROUPR0                   (GIC_GICR_BASE + 0x10080)
#define GIC_GICR_ISENABLER0                 (GIC_GICR_BASE + 0x10100)
#define GIC_GICR_ICENABLER0                 (GIC_GICR_BASE + 0x10180)
#define GIC_GICR_ISPENDR0                   (GIC_GICR_BASE + 0x10200)
#define GIC_GICR_ICPENDR0                   (GIC_GICR_BASE + 0x10280)
#define GIC_GICR_ISACTIVER0                 (GIC_GICR_BASE + 0x10300)
#define GIC_GICR_ICACTIVER0                 (GIC_GICR_BASE + 0x10380)
#define GIC_GICR_IPRIORITYR(N)              (GIC_GICR_BASE + 0x10400 + (N) * 4)
#define GIC_GICR_ICFGR0                     (GIC_GICR_BASE + 0x10C00)
#define GIC_GICR_ICFGR1                     (GIC_GICR_BASE + 0x10C04)
#define GIC_GICR_IGRPMODR0                  (GIC_GICR_BASE + 0x10D00)
#define GIC_GICR_NSACR                      (GIC_GICR_BASE + 0x10E00)

#define REG_GIC_GICR_CTLR                   ((volatile uint32_t *)(uintptr_t)GIC_GICR_CTLR)
#define REG_GIC_GICR_IIDR                   ((volatile uint32_t *)(uintptr_t)GIC_GICR_IIDR)
#define REG_GIC_GICR_WAKER                  ((volatile uint32_t *)(uintptr_t)GIC_GICR_WAKER)
#define REG_GIC_GICR_IGROUPR0               ((volatile uint32_t *)(uintptr_t)GIC_GICR_IGROUPR0)
#define REG_GIC_GICR_ISENABLER0             ((volatile uint32_t *)(uintptr_t)GIC_GICR_ISENABLER0)
#define REG_GIC_GICR_ICENABLER0             ((volatile uint32_t *)(uintptr_t)GIC_GICR_ICENABLER0)
#define REG_GIC_GICR_ISPENDR0               ((volatile uint32_t *)(uintptr_t)GIC_GICR_ISPENDR0)
#define REG_GIC_GICR_ICPENDR0               ((volatile uint32_t *)(uintptr_t)GIC_GICR_ICPENDR0)
#define REG_GIC_GICR_ICACTIVER0             ((volatile uint32_t *)(uintptr_t)GIC_GICR_ICACTIVER0)
#define REG_GIC_GICR_IPRIORITYR(n)          ((volatile uint32_t *)(uintptr_t)GIC_GICR_IPRIORITYR(n))
#define REG_GIC_GICR_ICFGR0                 ((volatile uint32_t *)(uintptr_t)GIC_GICR_ICFGR0)
#define REG_GIC_GICR_ICFGR1                 ((volatile uint32_t *)(uintptr_t)GIC_GICR_ICFGR1)

#define GIC_INT_MAX                         QEMU_VIRT_GIC_INT_MAX

#define GICR_WAKER_ProcessorSleep_BIT       (0x2U)
#define GICR_WAKER_ChildrenASleep_BIT       (0x4U)
#define GIC_GICR_INTPRIORITY_PER_REG        4
#define GIC_GICR_INTPRIORITY_SIZE_PER_REG   8
#define GIC_GICR_ICFGR_PER_REG              16
#define GIC_GICR_ICFGR_BITS_PER_REG         2
#define GIC_GICR_INT_PER_REG                32
#define GIC_GICR_ICPENDR_PER_REG            32
#define GIC_GICR_ISPENDR_PER_REG            32
#define GIC_GICR_ICENABLER_PER_REG          32
#define GIC_GICR_ISENABLER_PER_REG          32
#define GIC_SGI_MAX							16

#define DAIF_IRQ_BIT    (1<<1)

static inline void enable_irq(void)
{
    __asm__ __volatile("msr DAIFClr, %0\n\t" : : "i" (DAIF_IRQ_BIT) : "memory");
}

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

static inline uint64_t mmio_read64(uint64_t addr)
{
    volatile uint64_t *reg = (volatile uint64_t *)addr;
    return *reg;
}

static inline void mmio_write64(uint64_t addr, uint64_t val)
{
    volatile uint64_t *reg = (volatile uint64_t *)addr;
    *reg = val;
}

/* 初始化 GICv3（主 CPU，需 distributor 权限） */
void gicv3_init(void);

/* 每个 CPU 的 GIC CPU 接口初始化 */
void gicv3_init_cpu(void);

/* 读取中断应答寄存器 */
uint32_t gicv3_read_iar(void);

/* 写入中断结束寄存器 */
void gicv3_write_eoir(uint32_t iar);

/* 释放中断（deactivate） */
void gicv3_write_dir(uint32_t irq_id);

/* 设置当前 CPU 的优先级掩码 */
void gicv3_set_pmr(uint8_t pmr);

/* 获取最高优先级挂起中断 */
uint32_t gicv3_get_highest_priority_pending(void);

/* 设置 SPI 的目标 CPU（亲和性路由） */
void gicv3_set_irq_target(uint32_t irq_id, uint8_t cpu_id);

/* 使能/禁用特定 IRQ */
void gicv3_enable_irq(uint32_t irq_id, bool enable);

/* 检查 IRQ 是否属于当前 VM（当前全直通，返回 true） */
bool gicv3_irq_belongs_to_vm(uint32_t irq_id);

/* 处理 Hypervisor 自身的中断（IPI、定时器等） */
void gicv3_handle_irq(void);

/* 维护中断处理入口（需要在向量表中调用） */
void gicv3_maintenance_handler(void);

uint8_t gicv3_get_physical_priority(uint32_t irq_id);

void gicv3_route_irq_to_el2(uint32_t irq_id);

#define GICH_HCR_LRENPIE_BIT   				(1U << 2)
#define ICH_HCR_LRENPIE_BIT    				GICH_HCR_LRENPIE_BIT

#define icc_bpr1_el1    					S3_0_C12_C12_3
#define icc_ctlr_el1    					S3_0_C12_C12_4
#define icc_igrpen1_el1 					S3_0_C12_C12_7
#define ich_hcr_el2     					S3_4_C12_C11_0
#define icc_iar1_el1    					S3_0_C12_C12_0
#define icc_eoir1_el1   					S3_0_C12_C12_1
#define icc_dir_el1     					S3_0_C12_C11_1

#define STR(s)    #s
#define XSTR(s)   STR(s)

#define SYSREG_GEN_ACCESSORS_NAME(reg, name)                          \
    static inline unsigned long sysreg##reg##read()                   \
    {                                                                 \
        unsigned long _temp;                                          \
        __asm__ volatile("mrs %0, " XSTR(name) "\n\r" : "=r"(_temp)); \
        return _temp;                                                 \
    }                                                                 \
    static inline void sysreg##reg##write(unsigned long val)          \
    {                                                                 \
        __asm__ volatile("msr " XSTR(name) ", %0\n\r" ::"r"(val));    \
    }

#define SYSREG_GEN_ACCESSORS(reg) SYSREG_GEN_ACCESSORS_NAME(_##reg##_, reg)

SYSREG_GEN_ACCESSORS(icc_bpr1_el1)
SYSREG_GEN_ACCESSORS(icc_ctlr_el1)
SYSREG_GEN_ACCESSORS(icc_igrpen1_el1)
SYSREG_GEN_ACCESSORS(ich_hcr_el2)
SYSREG_GEN_ACCESSORS(icc_iar1_el1)
SYSREG_GEN_ACCESSORS(icc_eoir1_el1)
SYSREG_GEN_ACCESSORS(icc_dir_el1)

static inline uint32_t gicc_iar()
{
    return (uint32_t)sysreg_icc_iar1_el1_read();
}

static inline void gicc_eoir(uint32_t eoir)
{
    sysreg_icc_eoir1_el1_write(eoir);
}

static inline void gicc_dir(uint32_t dir)
{
    sysreg_icc_dir_el1_write(dir);
}

/* ICC_CTLR_EL1: Interrupt Controller Control Register (EL1) */
static inline void icc_write_icc_ctlr(uint32_t val)
{
    __asm__ volatile("msr S3_0_C12_C12_4, %0" : : "r"((uint64_t)val));
    __asm__ volatile("isb");
}

/* ICC_PMR_EL1: Priority Mask Register */
static inline void icc_write_icc_pmr(uint32_t val)
{
    __asm__ volatile("msr S3_0_C4_C6_0, %0" : : "r"((uint64_t)val));
    __asm__ volatile("isb");
}

/* ICC_BPR1_EL1: Binary Point Register 1 */
static inline void icc_write_icc_bpr1(uint32_t val)
{
    __asm__ volatile("msr S3_0_C12_C12_3, %0" : : "r"((uint64_t)val));
    __asm__ volatile("isb");
}

static inline uint64_t read_icc_sre_el2(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, icc_sre_el2" : "=r"(val));
    return val;
}

static inline void write_icc_sre_el2(uint64_t val) {
    __asm__ volatile("msr icc_sre_el2, %0" :: "r"(val));
}

static inline void write_icc_pmr_el2(uint64_t val) {
    __asm__ volatile("msr S3_4_C4_C6_0, %0" :: "r"(val));
    __asm__ volatile("isb");
}

static inline void write_icc_igrpen0_el2(uint64_t val) {
    __asm__ volatile("msr S3_4_C12_C12_6, %0" :: "r"(val));
    __asm__ volatile("isb");
}

static inline void write_icc_igrpen1_el2(uint64_t val) {
    __asm__ volatile("msr S3_4_C12_C12_7, %0" :: "r"(val));
    __asm__ volatile("isb");
}

static inline uint32_t read_ich_hcr_el2(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, S3_4_C12_C11_0" : "=r"(val));
    return (uint32_t)val;
}

static inline void write_ich_hcr_el2(uint32_t val) {
    __asm__ volatile("msr S3_4_C12_C11_0, %0" : : "r"((uint64_t)val));
}

#if 0
static inline uint64_t read_ich_hcr_el2(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, ich_hcr_el2" : "=r"(val));
    return val;
}

static inline void write_ich_hcr_el2(uint64_t val) {
    __asm__ volatile("msr ich_hcr_el2, %0" :: "r"(val));
}
#endif

// ICH_VTR_EL2: 虚拟化特性寄存器（查LR数量）
static inline uint64_t read_ich_vtr_el2(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, ich_vtr_el2" : "=r"(val));
    return val;
}

// ICH_MISR_EL2: 维护中断状态寄存器
static inline uint64_t read_ich_misr_el2(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, ich_misr_el2" : "=r"(val));
    return val;
}

// ICH_VMCR_EL2: 虚拟机控制寄存器（Guest中断全局配置）
static inline uint64_t read_ich_vmcr_el2(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, ich_vmcr_el2" : "=r"(val));
    return val;
}

static inline void write_ich_vmcr_el2(uint64_t val) {
    __asm__ volatile("msr ich_vmcr_el2, %0" :: "r"(val));
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

#if 0
// ICH_LR<n>_EL2: 列表寄存器（注入虚拟中断用，QEMU最多16个）
#define ICH_LR_EL2(n) ({ \
    uint64_t val; \
    __asm__ volatile("mrs %0, ich_lr" #n "_el2" : "=r"(val)); \
    val; \
})

#define WRITE_ICH_LR_EL2(n, val) \
    __asm__ volatile("msr ich_lr" #n "_el2, %0" :: "r"(val))

// 通用LR读写函数（动态索引）
static inline uint64_t read_ich_lr_el2(unsigned int lr) {
    switch (lr & 0xf) {
        case 0: return ICH_LR_EL2(0);
        case 1: return ICH_LR_EL2(1);
        case 2: return ICH_LR_EL2(2);
        case 3: return ICH_LR_EL2(3);
        case 4: return ICH_LR_EL2(4);
        case 5: return ICH_LR_EL2(5);
        case 6: return ICH_LR_EL2(6);
        case 7: return ICH_LR_EL2(7);
        case 8: return ICH_LR_EL2(8);
        case 9: return ICH_LR_EL2(9);
        case 10: return ICH_LR_EL2(10);
        case 11: return ICH_LR_EL2(11);
        case 12: return ICH_LR_EL2(12);
        case 13: return ICH_LR_EL2(13);
        case 14: return ICH_LR_EL2(14);
        case 15: return ICH_LR_EL2(15);
        default: return 0;
    }
}

static inline void write_ich_lr_el2(unsigned int lr, uint64_t val) {
    switch (lr & 0xf) {
        case 0: WRITE_ICH_LR_EL2(0, val); break;
        case 1: WRITE_ICH_LR_EL2(1, val); break;
        case 2: WRITE_ICH_LR_EL2(2, val); break;
        case 3: WRITE_ICH_LR_EL2(3, val); break;
        case 4: WRITE_ICH_LR_EL2(4, val); break;
        case 5: WRITE_ICH_LR_EL2(5, val); break;
        case 6: WRITE_ICH_LR_EL2(6, val); break;
        case 7: WRITE_ICH_LR_EL2(7, val); break;
        case 8: WRITE_ICH_LR_EL2(8, val); break;
        case 9: WRITE_ICH_LR_EL2(9, val); break;
        case 10: WRITE_ICH_LR_EL2(10, val); break;
        case 11: WRITE_ICH_LR_EL2(11, val); break;
        case 12: WRITE_ICH_LR_EL2(12, val); break;
        case 13: WRITE_ICH_LR_EL2(13, val); break;
        case 14: WRITE_ICH_LR_EL2(14, val); break;
        case 15: WRITE_ICH_LR_EL2(15, val); break;
    }
}
#endif

static inline void gicv3_set_irq_priority(uint32_t irq, uint8_t prio) {
    uint32_t n = irq / 4;
    uint32_t shift = (irq % 4) * 8;
    uint32_t val = mmio_read32(GICD_BASE + GICD_IPRIORITYR(n));
    val &= ~(0xFF << shift);
    val |= (prio << shift);
    mmio_write32(GICD_BASE + GICD_IPRIORITYR(n), val);
}

static inline void gicv3_send_sgi(uint32_t sgi_id, uint32_t cpu_mask) {
    mmio_write32(GICD_BASE + GICD_SGIR, (cpu_mask << 16) | sgi_id);
}

void gicr_sgi_config(uint32_t irq, uint32_t cfg);
void gicr_ppi_config(uint32_t irq, uint32_t cfg);
void gicr_set_priority(uint32_t irq, uint32_t pri);
void gicr_clear_pending(uint32_t irq);
void gicr_enable_irq(uint32_t irq);
void gicr_disable_irq(uint32_t irq);

#endif /* GICV3_H */