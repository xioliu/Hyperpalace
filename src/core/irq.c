#include "hp_types.h"
#include "vm.h"
#include "hp_config.h"
#include "irq.h"
#include "arch_ops.h"

void hp_irq_init(void)
{
    /* 架构/平台初始化已完成 */
}

void hp_irq_dispatch(uint32_t irq_id)
{
    if (irq_id >= HP_IRQ_MAX) {
        return;
    }

    /* 检查中断是否应注入当前 VM */
    if (!hp_irq_is_for_current_vm(irq_id)) {
        return;
    }

    /* 获取物理优先级（若架构支持） */
    uint8_t priority = 0x80U;
    if ((g_arch_ops != NULL) && (g_arch_ops->get_physical_priority != NULL)) {
        priority = g_arch_ops->get_physical_priority(irq_id);
    }

    /* 注入虚拟中断，传递优先级 */
    if ((g_arch_ops != NULL) && (g_arch_ops->irq_inject != NULL)) {
        g_arch_ops->irq_inject(irq_id, priority);
    }
}

bool hp_irq_is_for_current_vm(uint32_t irq_id)
{
    hp_vcpu_id_t vcpu_id = hp_vcpu_get_current();
    if (vcpu_id == HP_INVALID_VCPU_ID) return false;

    hp_vm_id_t vm_id = hp_vcpu_get_vm_id(vcpu_id);
    if (vm_id == HP_INVALID_VM_ID) return false;

    return hp_vm_is_irq_assigned(vm_id, irq_id);
}