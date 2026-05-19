/**
 * @file src/arch/armv8/armv8_mmu.c
 * @brief ARMv8 Stage-2 MMU 实现 (MISRA C:2012 合规)
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "hp_types.h"
#include "vm.h"
#include "hp_config.h"
#include "armv8_mmu_priv.h"
#include "arch_ops.h"
#include "errno.h"
#include "sysregs.h"
#include "uart.h"

/* Stage-2 翻译配置（从 VTCR_EL2 解析） */
struct stage2_config {
    uint32_t start_level;   /* 起始查找级别 */
    uint32_t max_level;     /* 最大级别（4KB 页所在的级别） */
    uint32_t tg0;           /* 粒度：0=4KB,1=64KB,2=16KB */
    uint32_t sl0;           /* SL0 原始值 */
    uint32_t t0sz;          /* T0SZ */
};

/* ========== 全局变量 ========== */

/* MMU 上下文（页表池、寄存器配置） */
armv8_mmu_ctx_t g_armv8_mmu_ctx = {0};

/* ========== 静态辅助函数声明 ========== */

static inline uint64_t align_to_page(uint64_t addr, uint64_t page_size);
static inline uint64_t align_down_to_page(uint64_t addr, uint64_t page_size);
static inline bool is_aligned(uint64_t addr, uint64_t alignment);
static void armv8_pt_pool_init(armv8_pt_pool_t *pool, void *start, size_t size);
static void* armv8_pt_pool_alloc(armv8_pt_pool_t *pool);
static void armv8_pt_pool_free(armv8_pt_pool_t *pool, void *table);
static uint64_t perm_to_stage2_attrs(uint32_t perm);
static int32_t stage2_map_range(uint64_t *pgd, uint64_t guest_pa, uint64_t host_pa,
                                uint64_t size, uint32_t perm);
static int32_t stage2_unmap_range(uint64_t *pgd, uint64_t guest_pa, uint64_t size);
static uint64_t* walk_stage2_table(uint64_t *pgd, uint64_t va, uint32_t *out_level,
                                   bool create);

/* ========== 辅助函数实现 ========== */

static inline uint64_t align_to_page(uint64_t addr, uint64_t page_size)
{
    return (addr + page_size - 1U) & ~(page_size - 1U);
}

static inline uint64_t align_down_to_page(uint64_t addr, uint64_t page_size)
{
    return addr & ~(page_size - 1U);
}

static inline bool is_aligned(uint64_t addr, uint64_t alignment)
{
    return (addr & (alignment - 1U)) == 0U;
}

/* 初始化静态页表池 */
static void armv8_pt_pool_init(armv8_pt_pool_t *pool, void *start, size_t size)
{
    if ((pool != NULL) && (start != NULL) && (size >= HP_PAGE_SIZE_4K)) {
        uintptr_t aligned_start = align_down_to_page((uintptr_t)start, HP_PAGE_SIZE_4K);
        size_t aligned_size = align_to_page(size, HP_PAGE_SIZE_4K);
        pool->pool_start = (void *)aligned_start;
        pool->pool_size = aligned_size;
        pool->allocated = 0U;
        pool->table_count = 0U;
        pool->initialized = true;
    }
}

/* 从池中分配一个页表页（4KB 对齐） */
static void* armv8_pt_pool_alloc(armv8_pt_pool_t *pool)
{
    void *table = NULL;

    if ((pool != NULL) && pool->initialized) {
        if ((pool->allocated + HP_PAGE_SIZE_4K) <= pool->pool_size) {
            if (pool->table_count < HP_MAX_PAGE_TABLES) {
                table = (uint8_t *)pool->pool_start + pool->allocated;
                pool->allocated += HP_PAGE_SIZE_4K;
                pool->table_count++;
                (void)memset(table, 0, HP_PAGE_SIZE_4K);
            }
        }
    }
    return table;
}

/* 释放页表页（当前仅减少计数，不回收物理内存） */
static void armv8_pt_pool_free(armv8_pt_pool_t *pool, void *table)
{
    (void)table;
    if ((pool != NULL) && pool->initialized) {
        if (pool->table_count > 0U) {
            pool->table_count--;
        }
    }
}

/* 将通用权限转换为 Stage-2 页表属性 */
static uint64_t perm_to_stage2_attrs(uint32_t perm)
{
    uint64_t attrs = 0U;

    /* MemAttr[3:0] = 0b0111 (Normal WB WA, Inner & Outer) */
    attrs |= (0x7U << 2);   // bits[5:2]

    /* 访问权限：EL1 读写 */
    attrs |= (3U << 6);   // AP[2:1] = 3

    /* 内部共享 */
    attrs |= (3U << 8);   // SH = 3

    /* 访问标志 */
    attrs |= (1U << 10);  // AF = 1

    /* 执行权限：只有明确禁止执行时才设置 XN */
    if ((perm & HP_MEM_EXEC) == 0U) {
        attrs |= (1ULL << 53);  // XN
        attrs |= (1ULL << 54);  // PXN
    }
    return attrs;
}

/* 获取指定地址和级别的页表索引 */
static uint32_t get_table_index(uint64_t addr, uint32_t level)
{
    uint32_t shift;
    switch (level) {
        case 0U: shift = 39U; break; /* Level 0：bits[39:30] (1GB 块) */
        case 1U: shift = 30U; break; /* Level 1：bits[29:21] (2MB 块) */
        case 2U: shift = 21U; break; /* Level 2：bits[20:12] (4KB 页) */
        case 3U: shift = 12U;  break; /* Level 3：bits[11:3] (如果使用) */
        default: return 0U;
    }
    return (uint32_t)((addr >> shift) & 0x1FFU);
}

/* 遍历 Stage-2 页表，返回目标地址的 PTE 指针，若 create 为 true 则创建中间表 */
static uint64_t* walk_stage2_table(uint64_t *pgd, uint64_t va, uint32_t *out_level,
                                   bool create)
{
    uint64_t *table = pgd;
    uint32_t level;

    for (level = 0U; level < 3U; level++) {
        uint32_t index = get_table_index(va, level);
        uint64_t *pte = &table[index];
        uint64_t desc = *pte;

        if ((desc & 0x3U) == 0U) {
            /* 描述符无效 */
            if (!create) {
                return NULL;
            }
            /* 分配下一级页表 */
            void *new_table = armv8_pt_pool_alloc(&g_armv8_mmu_ctx.pt_pool);
            if (new_table == NULL) {
                return NULL;
            }
            *pte = ((uint64_t)new_table & ARMV8_PTE_ADDR_MASK) | ARMV8_PTE_TYPE_TABLE;
            table = (uint64_t *)new_table;
        } else if ((desc & 0x3U) == ARMV8_PTE_TYPE_TABLE) {
            /* 表描述符 */
            table = (uint64_t *)(desc & ARMV8_PTE_ADDR_MASK);
        } else {
            /* 块/页描述符，提前结束 */
            break;
        }
    }

    if (out_level != NULL) {
        *out_level = level;
    }
    return &table[get_table_index(va, level)];
}

static void stage2_get_config(struct stage2_config *cfg)
{
    uint64_t vtcr = read_vtcr_el2();

    cfg->tg0  = (vtcr >> 14) & 0x3U;   /* TG0 */
    cfg->sl0  = (vtcr >> 6) & 0x3U;    /* SL0 */
    cfg->t0sz = vtcr & 0x3FU;          /* T0SZ */

    /* 根据 TG0 和 SL0 确定起始查找级别（简化，仅支持 4KB 粒度） */
    if (cfg->tg0 == 0U) { /* 4KB */
        switch (cfg->sl0) {
            case 0U: cfg->start_level = 2U; break; /* level 2 */
            case 1U: cfg->start_level = 1U; break; /* level 1 */
            case 2U: cfg->start_level = 0U; break; /* level 0 */
            case 3U: cfg->start_level = 3U; break; /* level 3 */
            default: cfg->start_level = 1U; break;
        }
        cfg->max_level = 3U;  /* 4KB 页表最大到 level 3 */
    } else {
        /* 其他粒度暂不支持，默认 */
        cfg->start_level = 2U;
        cfg->max_level = 3U;
    }
}

/* Stage-2 映射范围 */
/* 增强的 Stage‑2 映射函数 */
static int32_t stage2_map_range(uint64_t *pgd, uint64_t guest_pa, uint64_t host_pa,
                                uint64_t size, uint32_t perm)
{
    struct stage2_config cfg;
    stage2_get_config(&cfg);

    uint64_t va = guest_pa;
    uint64_t pa = host_pa;
    uint64_t remaining = size;
    uint64_t attrs = perm_to_stage2_attrs(perm);

    if (pgd == NULL) return HP_EINVAL;

    while (remaining > 0U) {
        /* 强制使用 4KB 页映射，避免 2MB 块索引重叠冲突 */
        uint64_t block_size = 0x1000ULL;
        uint32_t level = cfg.max_level;   // level = 3

        /* 遍历/创建中间级页表 */
        uint64_t *table = pgd;
        for (uint32_t lvl = cfg.start_level; lvl < level; lvl++) {
            uint32_t index = get_table_index(va, lvl);
            uint64_t *pte = &table[index];
            uint64_t desc = *pte;

            if ((desc & 0x3U) == 0U) {
                void *new_table = armv8_pt_pool_alloc(&g_armv8_mmu_ctx.pt_pool);
                if (new_table == NULL) {
                    return HP_ENOMEM;
                }
                *pte = ((uint64_t)new_table & ARMV8_PTE_ADDR_MASK) | ARMV8_PTE_TYPE_TABLE;
                table = (uint64_t *)new_table;
            } else if ((desc & 0x3U) == ARMV8_PTE_TYPE_TABLE) {
                table = (uint64_t *)(desc & ARMV8_PTE_ADDR_MASK);
            } else {
                return HP_EEXIST;      /* 冲突 */
            }
        }

        /* 最终 4KB 页描述符 */
        uint32_t final_index = get_table_index(va, level);
        uint64_t *final_pte = &table[final_index];
        if ((*final_pte & 0x3U) != 0U) {
            return HP_EEXIST;
        }

        if (level == 3U) {
            *final_pte = (pa & 0x0000FFFFFFFFF000ULL) | 0x3U | attrs;   // 页描述符
        } else if (level == 2U) {
            *final_pte = (pa & 0x0000FFFFC0000000ULL) | 0x3U | attrs;   // 2MB 块描述符（备用）
        }
        //*final_pte = (pa & 0x0000FFFFFFFFF000ULL) | ARMV8_PTE_TYPE_PAGE | attrs;

        va += block_size;
        pa += block_size;
        remaining -= block_size;
    }

    tlbi_vmalle1is();   /* 全局刷 TLB */
    return 0;
}

/* Stage-2 解除映射范围 */
static int32_t stage2_unmap_range(uint64_t *pgd, uint64_t guest_pa, uint64_t size)
{
    uint64_t va = align_down_to_page(guest_pa, HP_PAGE_SIZE_4K);
    uint64_t remaining = align_to_page(size, HP_PAGE_SIZE_4K);
    bool unmapped_any = false;

    if (pgd == NULL) {
        return HP_EINVAL;
    }

    while (remaining > 0U) {
        uint32_t level;
        uint64_t *pte = walk_stage2_table(pgd, va, &level, false);
        if (pte != NULL) {
            if ((*pte & 0x3U) != 0U) {
                *pte = 0U;
                unmapped_any = true;
            }
        }
        va += HP_PAGE_SIZE_4K;
        remaining -= HP_PAGE_SIZE_4K;
    }

    if (unmapped_any) {
        tlbi_vmalle1is();
    }

    return 0;
}

/* ========== 架构操作回调实现 ========== */

/* 早期初始化：页表池、VTCR_EL2、MAIR_EL2 */
void armv8_mmu_early_init(void)
{
    extern uint8_t __stage2_pool_start[];
    extern uint8_t __stage2_pool_end[];

    /* 初始化页表池 */
    armv8_pt_pool_init(&g_armv8_mmu_ctx.pt_pool,
                       __stage2_pool_start,
                       (size_t)((uintptr_t)__stage2_pool_end -
                                (uintptr_t)__stage2_pool_start));

    /* 配置 VTCR_EL2：PS=40bit, SL0=2 (起始级别1), T0SZ=24, 4KB 粒度, Inner WBWA */
    uint64_t vtcr = 0;
    vtcr |= (24U << 0);   // T0SZ = 24 → IPA 40位
    vtcr |= (1U << 6);    // SL0 = 1 → 起始级别 1
    vtcr |= (2U << 16);   // PS = 40-bit physical address
    vtcr |= (0U << 14);   // TG0 = 4KB
    vtcr |= (3U << 12);   // SH0 = Inner Shareable
    vtcr |= (1U << 10);   // ORGN0 = Normal WB RA WA
    vtcr |= (1U << 8);    // IRGN0 = Normal WB RA WA
    vtcr |= (1U << 31);   // RES1
    g_armv8_mmu_ctx.vtcr_el2 = vtcr;
    write_vtcr_el2(vtcr);

    /* 配置 MAIR_EL2 */
    uint64_t mair = 0;
    mair |= ((uint64_t)ARMV8_MAIR_ATTR0_DEVICE << 0U);
    mair |= ((uint64_t)ARMV8_MAIR_ATTR1_NORMAL << 8U);
    g_armv8_mmu_ctx.mair_el2 = mair;
    write_mair_el2(mair);

    g_armv8_mmu_ctx.stage2_enabled = false;
}

/* 分配 Stage-2 根页表 */
uint64_t armv8_mmu_alloc_pgd(void)
{
    void *pgd = armv8_pt_pool_alloc(&g_armv8_mmu_ctx.pt_pool);
    if (pgd == NULL) {
        return 0ULL;
    }
    /* 返回物理地址（假设 VA == PA） */
    return (uint64_t)pgd;
}

/* 释放 Stage-2 根页表 */
void armv8_mmu_free_pgd(uint64_t pgd_pa)
{
    armv8_pt_pool_free(&g_armv8_mmu_ctx.pt_pool, (void *)(uintptr_t)pgd_pa);
}

/* Stage-2 映射 */
int32_t armv8_mmu_map(uint64_t pgd_pa, uint64_t guest_pa, uint64_t host_pa,
                      uint64_t size, uint32_t perm)
{
    uint64_t *pgd = (uint64_t *)(uintptr_t)pgd_pa;
    return stage2_map_range(pgd, guest_pa, host_pa, size, perm);
}

/* Stage-2 解除映射 */
int32_t armv8_mmu_unmap(uint64_t pgd_pa, uint64_t guest_pa, uint64_t size)
{
    uint64_t *pgd = (uint64_t *)(uintptr_t)pgd_pa;
    return stage2_unmap_range(pgd, guest_pa, size);
}

/* Stage-2 权限修改 */
int32_t armv8_mmu_protect(uint64_t pgd_pa, uint64_t guest_pa, uint64_t size,
                          uint32_t perm)
{
    uint64_t *pgd = (uint64_t *)(uintptr_t)pgd_pa;
    uint64_t va = align_down_to_page(guest_pa, HP_PAGE_SIZE_4K);
    uint64_t remaining = align_to_page(size, HP_PAGE_SIZE_4K);
    uint64_t attrs = perm_to_stage2_attrs(perm);
    bool modified = false;

    if (pgd == NULL) {
        return HP_EINVAL;
    }

    while (remaining > 0U) {
        uint32_t level;
        uint64_t *pte = walk_stage2_table(pgd, va, &level, false);
        if (pte != NULL) {
            uint64_t desc = *pte;
            if ((desc & 0x3U) != 0U) {
                /* 保留地址部分，替换属性 */
                *pte = (desc & ARMV8_PTE_ADDR_MASK) |
                       (desc & 0x3U) |           /* 保留类型 */
                       attrs;
                modified = true;
            }
        }
        va += HP_PAGE_SIZE_4K;
        remaining -= HP_PAGE_SIZE_4K;
    }

    if (modified) {
        tlbi_vmalle1is();
    }

    return 0;
}

/* 检查物理地址是否属于 Hypervisor 保留区域 */
bool armv8_mmu_is_hyp_reserved(uint64_t pa, uint64_t size)
{
    uint64_t end = pa + size;
    extern uint8_t __hyp_text_start[];
    extern uint8_t __hyp_text_end[];
    extern uint8_t __hyp_data_start[];
    extern uint8_t __hyp_data_end[];
    extern uint8_t __hyp_bss_start[];
    extern uint8_t __hyp_bss_end[];
    extern uint8_t __stage2_pool_start[];
    extern uint8_t __stage2_pool_end[];

    const struct {
        uint64_t start;
        uint64_t end;
    } reserved_regions[] = {
        { (uint64_t)__hyp_text_start,      (uint64_t)__hyp_text_end      },
        { (uint64_t)__hyp_data_start,      (uint64_t)__hyp_data_end      },
        { (uint64_t)__hyp_bss_start,       (uint64_t)__hyp_bss_end       },
        { (uint64_t)__stage2_pool_start,   (uint64_t)__stage2_pool_end   },
        /* 可根据需要添加更多（如 GIC 寄存器、UART 等） */
    };

    for (uint32_t i = 0U; i < (sizeof(reserved_regions) / sizeof(reserved_regions[0])); i++) {
        uint64_t r_start = reserved_regions[i].start;
        uint64_t r_end   = reserved_regions[i].end;
        if ((pa < r_end) && (end > r_start)) {
            return true;
        }
    }
    return false;
}

/* 虚拟地址转物理地址（假设恒等映射） */
uint64_t armv8_va_to_pa(const void *va)
{
    /* 在 EL2 下，通常使用恒等映射 */
    return (uint64_t)(uintptr_t)va;
}

/* 物理地址转虚拟地址（假设恒等映射） */
void* armv8_pa_to_va(uint64_t pa)
{
    return (void *)(uintptr_t)pa;
}
