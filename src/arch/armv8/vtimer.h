#ifndef VTIMER_H
#define VTIMER_H

#define PTIMER_IRQ 26
#define VTIMER_IRQ 27
#define CNTHP_CTL_ENABLE ((0<<1) | (1<<0))

void timer_init(void);
void timer_init_cpu1(void);
void virt_timer_interrupt_handler(struct arch_regs* regs);
void timer_handler(void);
void el2_phys_timer_init(void);

static inline uint32_t raw_read_cntfrq_el0(void)
{
    uint32_t cntfrq_el0;

    __asm__ __volatile__("mrs %0, CNTFRQ_EL0\n\t" : "=r" (cntfrq_el0) : : "memory");
    return cntfrq_el0;
}

static inline uint64_t raw_read_cntpct_el0(void)
{
    uint64_t cntpct_el0;

    __asm__ __volatile__("mrs %0, CNTPCT_EL0\n\t" : "=r" (cntpct_el0) : : "memory");
    return cntpct_el0;
}

static inline void raw_write_cntval_el2(uint64_t cntval_el2)
{
    __asm__ __volatile__("msr CNTHP_CVAL_EL2, %0\n\t" : : "r" (cntval_el2) : "memory");
}

static inline uint32_t raw_read_cntv_ctl(void)
{
    uint32_t cntv_ctl;

    __asm__ __volatile__("mrs %0, CNTHP_CTL_EL2\n\t" : "=r" (cntv_ctl) : : "memory");
    return cntv_ctl;
}

static inline void disable_cntv(void)
{
    uint32_t cntv_ctl;

    cntv_ctl = raw_read_cntv_ctl();
    cntv_ctl &= ~CNTHP_CTL_ENABLE;
    __asm__ __volatile__("msr CNTHP_CTL_EL2, %0\n\t" : : "r" (cntv_ctl) : "memory");
}

static inline void enable_cntv(void)
{
    uint32_t cntv_ctl;

    cntv_ctl = raw_read_cntv_ctl();
    cntv_ctl |= CNTHP_CTL_ENABLE;
    __asm__ __volatile__("msr CNTHP_CTL_EL2, %0\n\t" : : "r" (cntv_ctl) : "memory");
}

// ==============================
// EL2 物理通用定时器寄存器（ARMv8 标准编码）
// ==============================
static inline uint64_t read_cntfrq_el0(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, S3_3_C14_C0_0" : "=r"(val));
    return val;
}

static inline uint64_t read_cntpct_el0(void)
{
    uint64_t val;
    __asm__ volatile("mrs %0, S3_3_C14_C0_1" : "=r"(val));
    return val;
}

static inline uint64_t read_cntp_tval_el2(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, S3_4_C14_C2_0" : "=r"(val));
    return val;
}

static inline void write_cntp_tval_el2(uint64_t val) {
    __asm__ volatile("msr S3_4_C14_C2_0, %0" :: "r"(val));
}

static inline uint64_t read_cntp_ctl_el2(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, S3_4_C14_C2_1" : "=r"(val));
    return val;
}

static inline void write_cntp_ctl_el2(uint64_t val) {
    __asm__ volatile("msr S3_4_C14_C2_1, %0" :: "r"(val));
}

#define read_sysreg(reg) ({ \
    uint64_t __val; \
    asm volatile("mrs %0, " #reg : "=r"(__val)); \
    __val; \
})

#define write_sysreg(reg, val) ({ \
    uint64_t __val = (val); \
    asm volatile("msr " #reg ", %0" :: "r"(__val) : "memory"); \
    isb(); \
})

// 虚拟定时器（Guest使用，完整组）
#define read_cntvct_el0()      read_sysreg(cntvct_el0)
#define read_cntv_cval_el0()   read_sysreg(cntv_cval_el0)
#define write_cntv_cval_el0(v) write_sysreg(cntv_cval_el0, v)
#define read_cntv_tval_el0()   read_sysreg(cntv_tval_el0)
#define write_cntv_tval_el0(v) write_sysreg(cntv_tval_el0, v)
#define read_cntv_ctl_el0()    read_sysreg(cntv_ctl_el0)
#define write_cntv_ctl_el0(v)  write_sysreg(cntv_ctl_el0, v)
#define write_cntvoff_el2(v)   write_sysreg(cntvoff_el2, v)

#endif