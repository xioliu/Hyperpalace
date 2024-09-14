#include "hp_types.h"
#include "vm.h"
#include "hp_config.h"
#include "memory.h"
#include "arch_ops.h"
#include "errno.h"

void hp_memory_init(void)
{
    if ((g_arch_ops != NULL) && (g_arch_ops->mmu_early_init != NULL)) {
        g_arch_ops->mmu_early_init();
    }
}

uint64_t hp_stage2_alloc_pgd(void)
{
    if ((g_arch_ops != NULL) && (g_arch_ops->mmu_alloc_pgd != NULL)) {
        return g_arch_ops->mmu_alloc_pgd();
    }
    return 0ULL;
}

void hp_stage2_free_pgd(uint64_t pgd_pa)
{
    if ((g_arch_ops != NULL) && (g_arch_ops->mmu_free_pgd != NULL)) {
        g_arch_ops->mmu_free_pgd(pgd_pa);
    }
}

int32_t hp_stage2_map(uint64_t pgd_pa, uint64_t guest_pa, uint64_t host_pa,
                      uint64_t size, uint32_t perm)
{
    /* 参数合法性检查 */
    if (size == 0U) {
        return -HP_EINVAL;
    }
    if (((guest_pa & 0xFFFU) != 0U) || ((host_pa & 0xFFFU) != 0U)) {
        return -HP_EINVAL;
    }

    if ((g_arch_ops != NULL) && (g_arch_ops->mmu_map != NULL)) {
        return g_arch_ops->mmu_map(pgd_pa, guest_pa, host_pa, size, perm);
    }
    return -HP_ENOSYS;
}

int32_t hp_stage2_unmap(uint64_t pgd_pa, uint64_t guest_pa, uint64_t size)
{
    if (size == 0U) {
        return -HP_EINVAL;
    }
    if ((guest_pa & 0xFFFU) != 0U) {
        return -HP_EINVAL;
    }

    if ((g_arch_ops != NULL) && (g_arch_ops->mmu_unmap != NULL)) {
        return g_arch_ops->mmu_unmap(pgd_pa, guest_pa, size);
    }
    return -HP_ENOSYS;
}

int32_t hp_stage2_protect(uint64_t pgd_pa, uint64_t guest_pa, uint64_t size,
                          uint32_t perm)
{
    if (size == 0U) {
        return -HP_EINVAL;
    }
    if ((guest_pa & 0xFFFU) != 0U) {
        return -HP_EINVAL;
    }

    if ((g_arch_ops != NULL) && (g_arch_ops->mmu_protect != NULL)) {
        return g_arch_ops->mmu_protect(pgd_pa, guest_pa, size, perm);
    }
    return -HP_ENOSYS;
}

bool hp_memory_is_hyp_reserved(uint64_t pa, uint64_t size)
{
    if ((g_arch_ops != NULL) && (g_arch_ops->mmu_is_hyp_reserved != NULL)) {
        return g_arch_ops->mmu_is_hyp_reserved(pa, size);
    }
    /* 默认保守策略：视为保留区域，禁止映射 */
    return true;
}

uint64_t hp_va_to_pa(const void *va)
{
    if ((g_arch_ops != NULL) && (g_arch_ops->va_to_pa != NULL)) {
        return g_arch_ops->va_to_pa(va);
    }
    /* 若未实现，假设恒等映射 */
    return (uint64_t)(uintptr_t)va;
}

void *hp_pa_to_va(uint64_t pa)
{
    if ((g_arch_ops != NULL) && (g_arch_ops->pa_to_va != NULL)) {
        return g_arch_ops->pa_to_va(pa);
    }
    return (void *)(uintptr_t)pa;
}