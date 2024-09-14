#include "hp_types.h"
#include "vm.h"
#include "armv8_vm_priv.h"
#include "armv8_mmu.h"
#include "errno.h"
#include "string.h"

/* 静态分配私有数据池 */
struct armv8_vm_arch g_armv8_vm_arch_pool[HP_CONFIG_MAX_VMS];
struct armv8_vcpu_arch g_armv8_vcpu_arch_pool[HP_CONFIG_MAX_VCPUS];

int32_t armv8_vm_init(hp_vm_id_t vm_id)
{
    struct armv8_vm_arch *arch = armv8_get_vm_arch(vm_id);
    if (arch == NULL) {
        return -HP_EINVAL;
    }
    /* 清零私有数据 */
    (void)memset(arch, 0, sizeof(*arch));
    
    /* 从核心层获取已分配的 Stage-2 根页表物理地址 */
    uint64_t pgd_pa = hp_vm_get_pgd_pa(vm_id);
    if (pgd_pa == 0ULL) {
        return -HP_EINVAL;   // 核心层尚未分配PGD
    }

    arch->vttbr_el2 = pgd_pa;
    arch->stage2_pgd_va = (uint64_t *)armv8_pa_to_va(pgd_pa);
    return 0;
}

void armv8_vm_destroy(hp_vm_id_t vm_id)
{
    struct armv8_vm_arch *arch = armv8_get_vm_arch(vm_id);
    if (arch != NULL) {
        (void)memset(arch, 0, sizeof(*arch));
    }
}

