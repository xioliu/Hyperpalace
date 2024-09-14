#ifndef ARMV8_MMU_PRIV_H
#define ARMV8_MMU_PRIV_H

#include <hyperpalace/memory.h>

/* Stage-2 页表级别 */
#define ARMV8_STAGE2_MAX_LEVEL  4U      /* 最大4级页表 */

/* Stage-2 页表项类型 */
#define ARMV8_PTE_TYPE_BLOCK    0x1U    /* 块映射 */
#define ARMV8_PTE_TYPE_TABLE    0x3U    /* 表映射 */
#define ARMV8_PTE_TYPE_PAGE     0x3U    /* 页面映射 */

/* Stage-2 页表项属性 (位偏移) */
#define ARMV8_PTE_ATTR_SHIFT    2U
#define ARMV8_PTE_AP_SHIFT      6U
#define ARMV8_PTE_SH_SHIFT      8U
#define ARMV8_PTE_AF_SHIFT      10U
#define ARMV8_PTE_XN_SHIFT      53U
#define ARMV8_PTE_PXN_SHIFT     54U
#define ARMV8_PTE_CONT_SHIFT    52U

/* Stage-2 内存属性索引 (MAIR_EL2) */
#define ARMV8_MAIR_ATTR0_SHIFT  0U
#define ARMV8_MAIR_ATTR1_SHIFT  8U
#define ARMV8_MAIR_ATTR2_SHIFT  16U
#define ARMV8_MAIR_ATTR3_SHIFT  24U
#define ARMV8_MAIR_ATTR4_SHIFT  32U
#define ARMV8_MAIR_ATTR5_SHIFT  40U
#define ARMV8_MAIR_ATTR6_SHIFT  48U
#define ARMV8_MAIR_ATTR7_SHIFT  56U

/* MAIR_EL2 内存类型编码 */
#define ARMV8_MAIR_DEVICE_nGnRnE    0x00U   /* 设备内存，严格顺序 */
#define ARMV8_MAIR_DEVICE_nGnRE     0x04U   /* 设备内存，非严格顺序 */
#define ARMV8_MAIR_DEVICE_GRE       0x08U   /* 设备内存，可重排 */
#define ARMV8_MAIR_NORMAL_NC        0x44U   /* 普通内存，非缓存 */
#define ARMV8_MAIR_NORMAL_WBWA      0xFFU   /* 普通内存，写回/写分配 */

/* 页表项大小 */
#define ARMV8_PTE_SIZE              8U      /* 8字节 */
#define ARMV8_PTES_PER_TABLE        512U    /* 512个表项 */

/* 物理地址掩码 */
#define ARMV8_PA_MASK               0x0000FFFFFFFFF000ULL  /* 48位物理地址 */
#define ARMV8_PTE_ADDR_MASK         0x0000FFFFFFFFF000ULL

/* MAIR 属性编码 */
#define ARMV8_MAIR_ATTR0_DEVICE 0x00U   /* Device-nGnRnE */
#define ARMV8_MAIR_ATTR1_NORMAL 0xFFU   /* Normal WBWA */

/* 页表池最大页表数 */
#define HP_MAX_PAGE_TABLES      64U

/* 链接器符号声明 */
extern uint8_t __stage2_pool_start[];
extern uint8_t __stage2_pool_end[];

/* 静态页表池管理结构 */
typedef struct {
    void *pool_start;           /* 池起始地址 */
    size_t pool_size;           /* 池总大小 */
    size_t allocated;           /* 已分配大小 */
    uint32_t table_count;       /* 已分配的页表数量 */
    bool initialized;           /* 初始化标志 */
} armv8_pt_pool_t;

/* ARMv8 MMU 私有数据 */
typedef struct {
    armv8_pt_pool_t pt_pool;    /* 页表池 */
    uint64_t vttbr_el2;         /* 当前VTTBR_EL2值 */
    uint64_t vtcr_el2;          /* 当前VTCR_EL2值 */
    uint64_t mair_el2;          /* 内存属性 */
    bool stage2_enabled;        /* Stage-2 MMU是否使能 */
} armv8_mmu_ctx_t;

extern armv8_mmu_ctx_t g_armv8_mmu_ctx;

#endif /* ARMV8_MMU_PRIV_H */