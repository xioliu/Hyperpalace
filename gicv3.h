#ifndef  __GICV3_H
#define __GICV3_H

#define GIC_BASE                            QEMU_VIRT_GIC_BASE
#define GIC_INT_MAX                         QEMU_VIRT_GIC_INT_MAX
#define GIC_PRIO_MAX                        QEMU_VIRT_GIC_PRIO_MAX
#define GIC_INTNO_SGI0                      QEMU_VIRT_GIC_INTNO_SGIO
#define GIC_INTNO_PPI0                      QEMU_VIRT_GIC_INTNO_PPIO
#define GIC_INTNO_SPI0                      QEMU_VIRT_GIC_INTNO_SPIO

#define TIMER_IRQ                           27

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

#define GIC_GICD_ICFGR_LEVEL                0x0
#define GIC_GICD_ICFGR_EDGE                 0x1
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

#define REG_GIC_GICC_CTLR                   ((volatile uint32_t *)(uintptr_t)GIC_GICC_CTLR)
#define REG_GIC_GICC_PMR                    ((volatile uint32_t *)(uintptr_t)GIC_GICC_PMR)

void gicd_init(void);
void gicc_init(void);
void gicd_irq_config(uint32_t irq, uint32_t cfg);
void gicd_set_priority(uint32_t irq, uint32_t pri);
void gicd_set_target(uint32_t irq, uint32_t pe_nr);
void gicd_clear_pending(uint32_t irq);
void gicd_enable_irq(uint32_t irq);
void gicd_disable_irq(uint32_t irq);
void gic_init(void);
void irq_handle(exception_t *excp __attribute__((unused)));

#endif