#ifndef  __GICV3_H
#define __GICV3_H

#define GIC_BASE                            QEMU_VIRT_GIC_BASE
#define GIC_SGI_MAX							16
#define GIC_PPI_MAX							32
#define GIC_INT_MAX                         QEMU_VIRT_GIC_INT_MAX
#define GIC_PRIO_MAX                        QEMU_VIRT_GIC_PRIO_MAX
#define GIC_INTNO_SGI0                      QEMU_VIRT_GIC_INTNO_SGIO
#define GIC_INTNO_PPI0                      QEMU_VIRT_GIC_INTNO_PPIO
#define GIC_INTNO_SPI0                      QEMU_VIRT_GIC_INTNO_SPIO

#define TIMER_IRQ                           26

#define GIC_GICD_BASE                       GIC_BASE
#define GIC_GICC_BASE                       (GIC_GICD_BASE + 0x10000)
#define GIC_GICH_BASE                       (GIC_GICD_BASE + 0x30000)
#define GIC_GICV_BASE                       (GIC_GICD_BASE + 0x40000)
#define GIC_GICR_BASE                       (GIC_GICD_BASE + 0xA0000)

#define GIC_GICD_INT_PER_REG                32
#define GIC_GICD_INTPRIORITY_PER_REG        4
#define GIC_GICD_INTPRIORITY_SIZE_PER_REG   8
#define GIC_GICD_ICFGR_PER_REG              16
#define GIC_GICD_ICFGR_BITS_PER_REG         2
#define GIC_GICD_ITARGETSR_PER_REG          4
#define GIC_GICD_ITARGETSR_SIZE_PER_REG     8
#define GIC_GICD_ICPENDR_PER_REG            32
#define GIC_GICD_ISPENDR_PER_REG            32
#define GIC_GICD_ICENABLER_PER_REG          32
#define GIC_GICD_ISENABLER_PER_REG          32

/*
Table 11-25 Distributor register map of IHI0069F_gic_architecture_specification.pdf
*/
#define GIC_GICD_CTLR                       (GIC_GICD_BASE + 0x00)
#define GIC_GICD_TYPER                      (GIC_GICD_BASE + 0x04)
#define GIC_GICD_IIDR                       (GIC_GICD_BASE + 0x08)
#define GIC_GICD_TYPER2                     (GIC_GICD_BASE + 0x0c)
#define GIC_GICD_STATUSR                    (GIC_GICD_BASE + 0x10)
#define GIC_GICD_SETSPI_NSR                 (GIC_GICD_BASE + 0x40)
#define GIC_GICD_CLRSPI_NSR                 (GIC_GICD_BASE + 0x48)
#define GIC_GICD_SETSPI_SR                  (GIC_GICD_BASE + 0x50)
#define GIC_GICD_CLRSPI_SR                  (GIC_GICD_BASE + 0x58)
#define GIC_GICD_IGROUPR(N)                 (GIC_GICD_BASE + 0x080 + (N) * 4)
#define GIC_GICD_ISENABLER(N)               (GIC_GICD_BASE + 0x100 + (N) * 4)
#define GIC_GICD_ICENABLER(N)               (GIC_GICD_BASE + 0x180 + (N) * 4)
#define GIC_GICD_ISPENDR(N)                 (GIC_GICD_BASE + 0x200 + (N) * 4)
#define GIC_GICD_ICPENDR(N)                 (GIC_GICD_BASE + 0x280 + (N) * 4)
#define GIC_GICD_ISACTIVER(N)               (GIC_GICD_BASE + 0x300 + (N) * 4)
#define GIC_GICD_ICACTIVER(N)               (GIC_GICD_BASE + 0x380 + (N) * 4)
#define GIC_GICD_IPRIORITYR(N)              (GIC_GICD_BASE + 0x400 + (N) * 4)
#define GIC_GICD_ITARGETSR(N)               (GIC_GICD_BASE + 0x800 + (N) * 4)
#define GIC_GICD_ICFGR(N)                   (GIC_GICD_BASE + 0xC00 + (N) * 4)
#define GIC_GICD_IGRPMODR(N)                (GIC_GICD_BASE + 0xD00 + (N) * 4)
#define GIC_GICD_NSACR(N)                   (GIC_GICD_BASE + 0xD00 + (N) * 4)
#define GIC_GICD_SGIR                       (GIC_GICD_BASE + 0xF00)
#define GIC_GICD_CPENDSGIR(N)               (GIC_GICD_BASE + 0xF10 + (N) * 4)
#define GIC_GICD_SPENDSGIR(N)               (GIC_GICD_BASE + 0xF20 + (N) * 4)

#define REG_GIC_GICD_CTLR                   ((volatile uint32_t *)(uintptr_t)GIC_GICD_CTLR)
#define REG_GIC_GICD_TYPER                  ((volatile uint32_t *)(uintptr_t)GIC_GICD_TYPE)
#define REG_GIC_GICD_IIDR                   ((volatile uint32_t *)(uintptr_t)GIC_GICD_IIDR)
#define REG_GIC_GICD_TYPER2                 ((volatile uint32_t *)(uintptr_t)GIC_GICD_TYPE2)
#define REG_GIC_GICD_STATUSR                ((volatile uint32_t *)(uintptr_t)GIC_GICD_STATUSR)
#define REG_GIC_GICD_SETSPI_NSR             ((volatile uint32_t *)(uintptr_t)GIC_GICD_SETSPI_NSR)
#define REG_GIC_GICD_CLRSPI_NSR             ((volatile uint32_t *)(uintptr_t)GIC_GICD_CLRSPI_NSR)
#define REG_GIC_GICD_SETSPI_SR              ((volatile uint32_t *)(uintptr_t)GIC_GICD_SETSPI_SR)
#define REG_GIC_GICD_CLRSPI_SR              ((volatile uint32_t *)(uintptr_t)GIC_GICD_CLRSPI_SR)
#define REG_GIC_GICD_IGROUPR(n)             ((volatile uint32_t *)(uintptr_t)GIC_GICD_IGROUPR(n))
#define REG_GIC_GICD_ISENABLER(n)           ((volatile uint32_t *)(uintptr_t)GIC_GICD_ISENABLER(n))
#define REG_GIC_GICD_ICENABLER(n)           ((volatile uint32_t *)(uintptr_t)GIC_GICD_ICENABLER(n))
#define REG_GIC_GICD_ISPENDR(n)             ((volatile uint32_t *)(uintptr_t)GIC_GICD_ISPENDR(n))
#define REG_GIC_GICD_ICPENDR(n)             ((volatile uint32_t *)(uintptr_t)GIC_GICD_ICPENDR(n))
#define REG_GIC_GICD_ISACTIVER(n)           ((volatile uint32_t *)(uintptr_t)GIC_GICD_ISACTIVER(n))
#define REG_GIC_GICD_ICACTIVER(n)           ((volatile uint32_t *)(uintptr_t)GIC_GICD_ICACTIVER(n))
#define REG_GIC_GICD_IPRIORITYR(n)          ((volatile uint32_t *)(uintptr_t)GIC_GICD_IPRIORITYR(n))
#define REG_GIC_GICD_ITARGETSR(n)           ((volatile uint32_t *)(uintptr_t)GIC_GICD_ITARGETSR(n))
#define REG_GIC_GICD_ICFGR(n)               ((volatile uint32_t *)(uintptr_t)GIC_GICD_ICFGR(n))
#define REG_GIC_GICD_IGRPMODR(n)            ((volatile uint32_t *)(uintptr_t)GIC_GICD_IGRPMODR(n))
#define REG_GIC_GICD_NSCAR(n)               ((volatile uint32_t *)(uintptr_t)GIC_GICD_NSCAR(n))
#define REG_GIC_GICD_SGIR                   ((volatile uint32_t *)(uintptr_t)GIC_GICD_SGIR)
#define REG_GIC_GICD_CPENDSGIR(n)           ((volatile uint32_t *)(uintptr_t)GIC_GICD_CPENDSGIR(n))
#define REG_GIC_GICD_SPENDSGIR(n)           ((volatile uint32_t *)(uintptr_t)GIC_GICD_SPENDSGIR(n))

#define GICD_CTLR_ENABLE                    0x1
#define GICD_CTLR_DISABLE                   0x0
#define GICD_CTLR_ENA_BIT         			0x2
#define GICD_CTLR_ARE_NS_BIT      			0x20

#define GIC_GICD_ICFGR_LEVEL                0x0
#define GIC_GICD_ICFGR_EDGE                 0x1
#define GIC_GICR_ICFGR_LEVEL                0x0
#define GIC_GICR_ICFGR_EDGE                 0x1
/*
Table 11-30 CPU interface register map of IHI0069F_gic_architecture_specification.pdf
*/
#define GIC_GICC_CTLR                       (GIC_GICC_BASE + 0x00)      /*CPU interface control register*/
#define GIC_GICC_PMR                        (GIC_GICC_BASE + 0x04)      /*Interrupt priority mask register*/
#define GIC_GICC_BPR                        (GIC_GICC_BASE + 0x08)      /*Binary point register*/
#define GIC_GICC_IAR                        (GIC_GICC_BASE + 0x0C)      /*Interrupt ack register*/
#define GIC_GICC_EOIR                       (GIC_GICC_BASE + 0x10)      /*End of interrupt register*/
#define GIC_GICC_RPR                        (GIC_GICC_BASE + 0x14)      /*Running priority register*/
#define GIC_GICC_HPPIR                      (GIC_GICC_BASE + 0x18)      /*Highest priority pending interrupt register*/
#define GIC_GICC_ABPR                       (GIC_GICC_BASE + 0x1C)      /*Aliased binary point register*/
#define GIC_GICC_AIAR                       (GIC_GICC_BASE + 0x20)      /*Aliased interrupt ack register*/
#define GIC_GICC_AEOIR                      (GIC_GICC_BASE + 0x24)      /*Aliased end of interrupt register*/
#define GIC_GICC_AHPPIR                     (GIC_GICC_BASE + 0x28)      /*Aliased highest priority pending interrupt register*/
#define GIC_GICC_STATUSR                    (GIC_GICC_BASE + 0x2C)      /*Aliased reporting status register*/
#define GIC_GICC_APR                        (GIC_GICC_BASE + 0xD0)      /*Active priorities register*/
#define GIC_GICC_NSAPR                      (GIC_GICC_BASE + 0xE0)      /*Non-secure active priorities register*/
#define GIC_GICC_IIDR                       (GIC_GICC_BASE + 0xFC)      /*CPU interface ID register*/
#define GIC_GICC_DIR                        (GIC_GICC_BASE + 0x1000)    /*Deactive interrupt register*/

#define GICC_CTLR_ENABLE                    0x1
#define GICC_CTLR_DISABLE                   0x0

#define GICC_IAR_ID_OFF         			0
#define GICC_IAR_ID_LEN 					24
#define GICC_IAR_ID_MSK          			0xFFFFFF

#define REG_GIC_GICC_CTLR                   ((volatile uint32_t *)(uintptr_t)GIC_GICC_CTLR)
#define REG_GIC_GICC_PMR                    ((volatile uint32_t *)(uintptr_t)GIC_GICC_PMR)

#define ICC_CTLR_EOIMode_BIT     			(0x1ULL << 1)
#define ICC_SGIR_SGIINTID_OFF    			24
#define ICC_SGIR_SGIINTID_LEN    			4
#define ICC_SGIR_SGIINTID(sgir)  			((sgir & 0x0f000000ULL) >> 24)
#define ICC_SGIR_IRM_BIT         			(1ULL << 40)
#define ICC_SGIR_TRGLSTFLT_OFF   			0
#define ICC_SGIR_TRGLSTFLT_LEN   			16
#define ICC_SGIR_TRGLSTFLT_MSK   			0xFFFF
#define ICC_SGIR_TRGLSTFLT(sgir) 			(sgir & 0xFFFF)
#define ICC_SGIR_AFF1_OFFSET     			(16)

#define ICC_SRE_ENB_BIT          			0x8
#define ICC_SRE_DIB_BIT          			0x4
#define ICC_SRE_DFB_BIT          			0x2
#define ICC_SRE_SRE_BIT          			0x1
#define ICC_IGRPEN_EL1_ENB_BIT   			0x1

#define GIC_GICR_INTPRIORITY_PER_REG        4
#define GIC_GICR_INTPRIORITY_SIZE_PER_REG   8
#define GIC_GICR_ICFGR_PER_REG              16
#define GIC_GICR_ICFGR_BITS_PER_REG         2
#define GIC_GICR_INT_PER_REG                32
#define GIC_GICR_ICPENDR_PER_REG            32
#define GIC_GICR_ISPENDR_PER_REG            32
#define GIC_GICR_ICENABLER_PER_REG          32
#define GIC_GICR_ISENABLER_PER_REG          32

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

#define GIC_GICR_ISPENDR_PER_REG            32

#define GICR_WAKER_ProcessorSleep_BIT       (0x2U)
#define GICR_WAKER_ChildrenASleep_BIT       (0x4U)

#define GICH_HCR_LRENPIE_BIT   				(1U << 2)
#define ICH_HCR_LRENPIE_BIT    				GICH_HCR_LRENPIE_BIT

#define GICH_VTR_OFF           				0
#define GICH_VTR_LEN           				6
#define GICH_VTR_MSK           				((1 << (GICH_VTR_LEN - GICH_VTR_OFF)) - 1)

#define ICH_VTR_OFF            				GICH_VTR_OFF
#define ICH_VTR_LEN            				GICH_VTR_LEN
#define ICH_VTR_MSK            				GICH_VTR_MSK

void gicd_init(void);
void gicR_init(void);
void gicc_init(void);

void gicd_irq_config(uint32_t irq, uint32_t cfg);
void gicd_set_priority(uint32_t irq, uint32_t pri);
void gicd_set_target(uint32_t irq, uint32_t pe_nr);
void gicd_clear_pending(uint32_t irq);
void gicd_enable_irq(uint32_t irq);
void gicd_disable_irq(uint32_t irq);

void gicr_sgi_config(uint32_t irq, uint32_t cfg);
void gicr_ppi_config(uint32_t irq, uint32_t cfg);
void gicr_set_priority(uint32_t irq, uint32_t pri);
void gicr_set_target(uint32_t irq, uint32_t pe_nr);
void gicr_clear_pending(uint32_t irq);
void gicr_enable_irq(uint32_t irq);
void gicr_disable_irq(uint32_t irq);

void gic_init(void);
void gic_handle(exception_t *excp __attribute__((unused)));

uint32_t gich_num_lrs(void);

static inline uint32_t gicc_iar()
{
    return (uint32_t)sysreg_icc_iar1_el1_read();
}

static inline void gich_write_lr(uint32_t i, uint64_t val)
{
    switch (i) {
        case 0:
            sysreg_ich_lr0_el2_write(val);
            break;
        case 1:
            sysreg_ich_lr1_el2_write(val);
            break;
        case 2:
            sysreg_ich_lr2_el2_write(val);
            break;
        case 3:
            sysreg_ich_lr3_el2_write(val);
            break;
        case 4:
            sysreg_ich_lr4_el2_write(val);
            break;
        case 5:
            sysreg_ich_lr5_el2_write(val);
            break;
        case 6:
            sysreg_ich_lr6_el2_write(val);
            break;
        case 7:
            sysreg_ich_lr7_el2_write(val);
            break;
        case 8:
            sysreg_ich_lr8_el2_write(val);
            break;
        case 9:
            sysreg_ich_lr9_el2_write(val);
            break;
        case 10:
            sysreg_ich_lr10_el2_write(val);
            break;
        case 11:
            sysreg_ich_lr11_el2_write(val);
            break;
        case 12:
            sysreg_ich_lr12_el2_write(val);
            break;
        case 13:
            sysreg_ich_lr13_el2_write(val);
            break;
        case 14:
            sysreg_ich_lr14_el2_write(val);
            break;
        case 15:
            sysreg_ich_lr15_el2_write(val);
            break;
        default:
            break;
    }
}

#endif
