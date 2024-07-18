#ifndef  __SYSREGS_H
#define __SYSREGS_H

#define icc_iar1_el1    					S3_0_C12_C12_0
#define icc_eoir1_el1   					S3_0_C12_C12_1
#define icc_dir_el1     					S3_0_C12_C11_1
#define icc_sre_el2                			S3_4_C12_C9_5
#define icc_pmr_el1     					S3_0_C4_C6_0
#define ich_vtr_el2     					S3_4_C12_C11_1
#define icc_hppir1_el1                      S3_0_C12_C12_2
#define icc_bpr1_el1    					S3_0_C12_C12_3
#define icc_ctlr_el1    					S3_0_C12_C12_4
#define icc_igrpen1_el1 					S3_0_C12_C12_7
#define ich_hcr_el2     					S3_4_C12_C11_0
#define ich_lr0_el2     					S3_4_C12_C12_0
#define ich_lr1_el2    						S3_4_C12_C12_1
#define ich_lr2_el2     					S3_4_C12_C12_2
#define ich_lr3_el2     					S3_4_C12_C12_3
#define ich_lr4_el2     					S3_4_C12_C12_4
#define ich_lr5_el2     					S3_4_C12_C12_5
#define ich_lr6_el2     					S3_4_C12_C12_6
#define ich_lr7_el2     					S3_4_C12_C12_7
#define ich_lr8_el2     					S3_4_C12_C13_0
#define ich_lr9_el2     					S3_4_C12_C13_1
#define ich_lr10_el2    					S3_4_C12_C13_2
#define ich_lr11_el2    					S3_4_C12_C13_3
#define ich_lr12_el2    					S3_4_C12_C13_4
#define ich_lr13_el2    					S3_4_C12_C13_5
#define ich_lr14_el2    					S3_4_C12_C13_6
#define ich_lr15_el2    					S3_4_C12_C13_7


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


SYSREG_GEN_ACCESSORS(hcr_el2)
SYSREG_GEN_ACCESSORS(icc_iar1_el1)
SYSREG_GEN_ACCESSORS(icc_eoir1_el1)
SYSREG_GEN_ACCESSORS(icc_dir_el1)
SYSREG_GEN_ACCESSORS(icc_sre_el2)
SYSREG_GEN_ACCESSORS(icc_pmr_el1)
SYSREG_GEN_ACCESSORS(ich_vtr_el2)
SYSREG_GEN_ACCESSORS(icc_hppir1_el1)
SYSREG_GEN_ACCESSORS(icc_bpr1_el1)
SYSREG_GEN_ACCESSORS(icc_ctlr_el1)
SYSREG_GEN_ACCESSORS(icc_igrpen1_el1)
SYSREG_GEN_ACCESSORS(ich_hcr_el2)
SYSREG_GEN_ACCESSORS(ich_lr0_el2)
SYSREG_GEN_ACCESSORS(ich_lr1_el2)
SYSREG_GEN_ACCESSORS(ich_lr2_el2)
SYSREG_GEN_ACCESSORS(ich_lr3_el2)
SYSREG_GEN_ACCESSORS(ich_lr4_el2)
SYSREG_GEN_ACCESSORS(ich_lr5_el2)
SYSREG_GEN_ACCESSORS(ich_lr6_el2)
SYSREG_GEN_ACCESSORS(ich_lr7_el2)
SYSREG_GEN_ACCESSORS(ich_lr8_el2)
SYSREG_GEN_ACCESSORS(ich_lr9_el2)
SYSREG_GEN_ACCESSORS(ich_lr10_el2)
SYSREG_GEN_ACCESSORS(ich_lr11_el2)
SYSREG_GEN_ACCESSORS(ich_lr12_el2)
SYSREG_GEN_ACCESSORS(ich_lr13_el2)
SYSREG_GEN_ACCESSORS(ich_lr14_el2)
SYSREG_GEN_ACCESSORS(ich_lr15_el2)

static inline void gicc_eoir(uint32_t eoir)
{
    sysreg_icc_eoir1_el1_write(eoir);
}

static inline void gicc_dir(uint32_t dir)
{
    sysreg_icc_dir_el1_write(dir);
}

#endif
