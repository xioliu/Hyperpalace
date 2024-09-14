#ifndef SYSREGS_H
#define SYSREGS_H

#include <stdint.h>

/* ========== HCR_EL2 位定义 ========== */
#define HCR_VM_BIT     (1U << 0)   /* Stage-2 MMU 使能 */
#define HCR_SWIO_BIT   (1U << 1)   /* 软件 I/O 指令陷阱 */
#define HCR_PTW_BIT    (1U << 2)   /* Stage-2 页表遍历陷阱 */
#define HCR_FMO_BIT    (1U << 3)   /* 物理 FIQ 路由到 EL2 */
#define HCR_IMO_BIT    (1U << 4)   /* 物理 IRQ 路由到 EL2 */
#define HCR_AMO_BIT    (1U << 5)   /* 物理 SError 路由到 EL2 */
#define HCR_TSC_BIT    (1U << 19)  /* 虚拟定时器陷阱 */
#define HCR_RW_BIT     (1U << 31)  /* EL1 执行状态：1 = AArch64 */

static inline uint64_t read_hcr_el2(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, hcr_el2" : "=r"(val));
    return val;
}
static inline void write_hcr_el2(uint64_t val) {
    __asm__ volatile("msr hcr_el2, %0" : : "r"(val));
    __asm__ volatile("isb");
}

static inline uint64_t read_vtcr_el2(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, vtcr_el2" : "=r"(val));
    return val;
}
static inline void write_vtcr_el2(uint64_t val) {
    __asm__ volatile("msr vtcr_el2, %0" : : "r"(val));
    __asm__ volatile("isb");
}

static inline uint64_t read_vttbr_el2(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, vttbr_el2" : "=r"(val));
    return val;
}
static inline void write_vttbr_el2(uint64_t val) {
    __asm__ volatile("msr vttbr_el2, %0" : : "r"(val));
    __asm__ volatile("isb");
}

static inline uint64_t read_mair_el2(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, mair_el2" : "=r"(val));
    return val;
}
static inline void write_mair_el2(uint64_t val) {
    __asm__ volatile("msr mair_el2, %0" : : "r"(val));
    __asm__ volatile("isb");
}

static inline uint64_t read_esr_el2(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, esr_el2" : "=r"(val));
    return val;
}
static inline uint64_t read_far_el2(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, far_el2" : "=r"(val));
    return val;
}

static inline uint64_t read_elr_el2(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, elr_el2" : "=r"(val));
    return val;
}
static inline void write_elr_el2(uint64_t val) {
    __asm__ volatile("msr elr_el2, %0" : : "r"(val));
}

static inline uint64_t read_spsr_el2(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, spsr_el2" : "=r"(val));
    return val;
}
static inline void write_spsr_el2(uint64_t val) {
    __asm__ volatile("msr spsr_el2, %0" : : "r"(val));
}

static inline uint64_t read_tpidr_el2(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, tpidr_el2" : "=r"(val));
    return val;
}
static inline void write_tpidr_el2(uint64_t val) {
    __asm__ volatile("msr tpidr_el2, %0" : : "r"(val));
}

static inline uint64_t read_mpidr_el1(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, mpidr_el1" : "=r"(val));
    return val;
}

static inline uint64_t read_par_el1(void)
{
    uint64_t val;
    __asm__ volatile("mrs %0, par_el1" : "=r"(val));
    return val;
}

static inline void at_s12e1r(uint64_t va)
{
    __asm__ volatile("at s12e1r, %0" : : "r"(va));
    __asm__ volatile("isb" ::: "memory");
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

/* TLB 维护 */
static inline void tlbi_vmalle1is(void) {
    __asm__ volatile("tlbi vmalle1is" ::: "memory");
    __asm__ volatile("dsb ish" ::: "memory");
    __asm__ volatile("isb" ::: "memory");
}

static inline void tlbi_ipas2e1is(uint64_t va) {
    __asm__ volatile("tlbi ipas2e1is, %0" : : "r"(va >> 12U));
    __asm__ volatile("dsb ish" ::: "memory");
    __asm__ volatile("isb" ::: "memory");
}

#endif /* SYSREGS_H */