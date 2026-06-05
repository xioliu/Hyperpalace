#ifndef ARMV8_VM_PRIV_H
#define ARMV8_VM_PRIV_H

#include <hyperpalace/vm.h>
#include <hyperpalace/hp_config.h>

/* 静态分配 ARMv8 私有数据池 */
extern struct armv8_vm_arch g_armv8_vm_arch_pool[HP_CONFIG_MAX_VMS];
extern struct armv8_vcpu_arch g_armv8_vcpu_arch_pool[HP_CONFIG_MAX_VCPUS];

/* 获取指定 ID 的架构私有指针 */
static inline struct armv8_vm_arch *armv8_get_vm_arch(hp_vm_id_t vm_id)
{
    if (vm_id < HP_CONFIG_MAX_VMS) {
        return &g_armv8_vm_arch_pool[vm_id];
    }
    return NULL;
}

static inline struct armv8_vcpu_arch *armv8_get_vcpu_arch(hp_vcpu_id_t vcpu_id)
{
    if (vcpu_id < HP_CONFIG_MAX_VCPUS) {
        return &g_armv8_vcpu_arch_pool[vcpu_id];
    }
    return NULL;
}

/* 检查物理地址是否属于 Hypervisor 保留区域（不可映射给 VM） */
bool armv8_is_hyp_reserved_region(uint64_t pa, uint64_t size);

void armv8_vcpu_run(hp_vcpu_id_t vcpu_id) HP_NORETURN;

#endif