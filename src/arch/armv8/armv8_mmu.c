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

/* ========== 全局变量 ========== */

/* MMU 上下文（页表池、寄存器配置） */
static armv8_mmu_ctx_t g_mmu_ctx = {0};

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

    /* 内存类型：设备或普通内存 */
    if ((perm & HP_MEM_DEVICE) != 0U) {
        attrs |= ((uint64_t)ARMV8_MAIR_ATTR0_DEVICE << ARMV8_PTE_ATTR_SHIFT);
    } else {
        attrs |= ((uint64_t)ARMV8_MAIR_ATTR1_NORMAL << ARMV8_PTE_ATTR_SHIFT);
    }

    /* 访问权限 */
    uint64_t ap = ((perm & HP_MEM_WRITE) != 0U) ? 0x3U : 0x2U;
    attrs |= (ap << ARMV8_PTE_AP_SHIFT);

    /* 可共享域 */
    attrs |= ((perm & HP_MEM_SHAREABLE) != 0U) ? (0x3U << ARMV8_PTE_SH_SHIFT)
                                                : (0x0U << ARMV8_PTE_SH_SHIFT);

    /* 访问标志 */
    attrs |= (1U << ARMV8_PTE_AF_SHIFT);

    /* 执行权限 */
    if ((perm & HP_MEM_EXEC) == 0U) {
        attrs |= (1ULL << ARMV8_PTE_XN_SHIFT);
        attrs |= (1ULL << ARMV8_PTE_PXN_SHIFT);
    }

    return attrs;
}

/* 获取指定地址和级别的页表索引 */
static uint32_t get_table_index(uint64_t addr, uint32_t level)
{
    uint32_t shift;
    switch (level) {
        case 0U: shift = 39U; break; /* 1GB */
        case 1U: shift = 30U; break; /* 2MB */
        case 2U: shift = 21U; break; /* 4KB */
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
            void *new_table = armv8_pt_pool_alloc(&g_mmu_ctx.pt_pool);
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

/* Stage-2 映射范围 */
static int32_t stage2_map_range(uint64_t *pgd, uint64_t guest_pa, uint64_t host_pa,
                                uint64_t size, uint32_t perm)
{
    uint64_t va = guest_pa;
    uint64_t pa = host_pa;
    uint64_t remaining = size;
    uint64_t attrs = perm_to_stage2_attrs(perm);

    if (pgd == NULL) {
        return -HP_EINVAL;
    }

    while (remaining > 0U) {
        /* 选择最大可能的块大小 */
        uint64_t block_size = HP_PAGE_SIZE_4K;

        if ((remaining >= HP_PAGE_SIZE_1G) && is_aligned(va, HP_PAGE_SIZE_1G) &&
            is_aligned(pa, HP_PAGE_SIZE_1G)) {
            block_size = HP_PAGE_SIZE_1G;
        } else if ((remaining >= HP_PAGE_SIZE_2M) && is_aligned(va, HP_PAGE_SIZE_2M) &&
                   is_aligned(pa, HP_PAGE_SIZE_2M)) {
            block_size = HP_PAGE_SIZE_2M;
        }

        /* 遍历/创建页表 */
        uint32_t final_level = 0U;
        uint64_t *pte = walk_stage2_table(pgd, va, &final_level, true);
        if (pte == NULL) {
            return -HP_ENOMEM;
        }

        /* 检查是否已存在映射 */
        if ((*pte & 0x3U) != 0U) {
            return -HP_EEXIST;
        }

        /* 创建块映射 */
        *pte = (pa & ARMV8_PTE_ADDR_MASK) | ARMV8_PTE_TYPE_BLOCK | attrs;

        va += block_size;
        pa += block_size;
        remaining -= block_size;
    }

    /* 刷新 TLB */
    tlbi_vmalle1is();

    return 0;
}

/* Stage-2 解除映射范围 */
static int32_t stage2_unmap_range(uint64_t *pgd, uint64_t guest_pa, uint64_t size)
{
    uint64_t va = align_down_to_page(guest_pa, HP_PAGE_SIZE_4K);
    uint64_t remaining = align_to_page(size, HP_PAGE_SIZE_4K);
    bool unmapped_any = false;

    if (pgd == NULL) {
        return -HP_EINVAL;
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
    /* 从链接脚本导入 Stage-2 页表池边界 */
    extern uint8_t __stage2_pool_start[];
    extern uint8_t __stage2_pool_end[];

    /* 初始化页表池 */
    armv8_pt_pool_init(&g_mmu_ctx.pt_pool,
                       __stage2_pool_start,
                       (size_t)((uintptr_t)__stage2_pool_end -
                                (uintptr_t)__stage2_pool_start));

    g_mmu_ctx.stage2_enabled = false;
}

/* 分配 Stage-2 根页表 */
uint64_t armv8_mmu_alloc_pgd(void)
{
    void *pgd = armv8_pt_pool_alloc(&g_mmu_ctx.pt_pool);
    if (pgd == NULL) {
        return 0ULL;
    }
    /* 返回物理地址（假设 VA == PA） */
    return (uint64_t)pgd;
}

/* 释放 Stage-2 根页表 */
void armv8_mmu_free_pgd(uint64_t pgd_pa)
{
    armv8_pt_pool_free(&g_mmu_ctx.pt_pool, (void *)(uintptr_t)pgd_pa);
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
        return -HP_EINVAL;
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
