/*
 * armv8_mmu.h - ARMv8 specific MMU definitions
 */

#ifndef ARMV8_MMU_H
#define ARMV8_MMU_H

#include <stdint.h>
#include <stdbool.h>
#include <hyperpalace/memory.h>

/* ARMv8 页表项类型 */
#define ARMv8_PTE_TYPE_BLOCK        (1ULL << 0)
#define ARMv8_PTE_TYPE_TABLE        (3ULL << 0)
#define ARMv8_PTE_TYPE_PAGE         (3ULL << 1)  /* 对于4KB页 */

/* Stage-2 页表项属性 (低12位) */
#define STAGE2_ATTR_MEMTYPE_NORMAL  (0xFFULL << 2)   /* Normal memory */
#define STAGE2_ATTR_MEMTYPE_DEVICE  (0x00ULL << 2)   /* Device-nGnRnE */
#define STAGE2_ATTR_SH_IS           (3ULL << 8)      /* Inner Shareable */
#define STAGE2_ATTR_AF              (1ULL << 10)     /* Access Flag */
#define STAGE2_ATTR_XN              (2ULL << 53)     /* Execute Never */

/* Stage-2 权限位 (S2AP) */
#define STAGE2_S2AP_READ            (1ULL << 6)
#define STAGE2_S2AP_WRITE           (1ULL << 7)
#define STAGE2_S2AP_RW              (STAGE2_S2AP_READ | STAGE2_S2AP_WRITE)

/* 初始化和配置 */
void armv8_mmu_early_init(void);
void armv8_mmu_enable_stage2(void);
void armv8_mmu_set_vttbr_el2(uint64_t vttbr);
void armv8_mmu_setup_vtcr_el2(void);

/* 页表分配 */
uint64_t armv8_mmu_alloc_pgd(void);
void armv8_mmu_free_pgd(uint64_t pgd_pa);

/* 映射操作 */
int32_t armv8_mmu_map(uint64_t pgd_pa, uint64_t guest_pa, uint64_t host_pa,
                      uint64_t size, uint32_t perm);
int32_t armv8_mmu_unmap(uint64_t pgd_pa, uint64_t guest_pa, uint64_t size);

/* 安全区域检查 */
bool armv8_mmu_is_hyp_reserved(uint64_t pa, uint64_t size);

/* 地址转换 */
uint64_t armv8_va_to_pa(const void *va);
void *armv8_pa_to_va(uint64_t pa);

/* 用于异常处理：处理Stage-2数据中止 */
bool armv8_handle_stage2_abort(uint64_t far, uint64_t esr);

#endif /* ARMV8_MMU_H */