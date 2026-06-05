/**
 * @file src/arch/armv8/armv8_vcpu.c
 * @brief ARMv8 vCPU运行实现，展示如何使用内存管理
 */
#include "hp_types.h"
#include "vm.h"
#include "hp_config.h"
#include "memory.h"
#include "arch_ops.h"
#include "errno.h"
#include "arch_ops.h"
#include "armv8_vm.h"
#include "armv8_vm_priv.h"
#include "sysregs.h"
#include "string.h"
#include "uart.h"
#include "vtimer.h"
#include "asm_defs.h"

/* 外部汇编入口 */
extern void armv8_vcpu_enter(struct armv8_vcpu_arch *ctx);

int32_t armv8_vcpu_init(hp_vcpu_id_t vcpu_id)
{
    struct armv8_vcpu_arch *arch = armv8_get_vcpu_arch(vcpu_id);
    if (arch == NULL) {
        return HP_EINVAL;
    }
    
    (void)memset(arch, 0, sizeof(*arch));
    /* 获取VM入口点 */
    hp_vm_id_t vm_id = hp_vcpu_get_vm_id(vcpu_id);
    uint64_t entry = hp_vm_get_entry(vm_id);
    
    /* 设置初始寄存器状态 */
    arch->regs.elr_el2 = entry;
    arch->regs.spsr_el2 = 0x3C5U;  /* EL1h, 异常屏蔽禁用 */
    arch->regs.sp_el1 = ((uint64_t)arch);

    vtimer_init(arch);
    
    return 0;
}

void armv8_vcpu_run(hp_vcpu_id_t vcpu_id)
{
    struct armv8_vcpu_arch *vcpu_arch  = armv8_get_vcpu_arch(vcpu_id);
    hp_vm_id_t vm_id = hp_vcpu_get_vm_id(vcpu_id);
    
    /* 致命错误：无法获取有效的 vCPU 或 VM */
    if ((vcpu_arch == NULL) || (vm_id == HP_INVALID_VM_ID)) {
        /* noreturn 函数不能返回，直接死循环或调用平台 panic */
        while (1) { __asm__ volatile("wfi"); }
    }

    /* 获取 Stage‑2 根页表物理地址 */
    uint64_t pgd_pa = hp_vm_get_pgd_pa(vm_id);
    if (pgd_pa == 0ULL) {
        while (1) { __asm__ volatile("wfi"); }
    }
    /* 设置 VTTBR_EL2 (VMID=1) */
    write_vttbr_el2(pgd_pa | (0x1ULL << 48));

    /* 设置系统寄存器 */
    //write_elr_el2(hp_vm_get_entry(vm_id));          /* Guest 入口 */
    //write_spsr_el2(0x3C5);              /* EL1h, DAIF 全开 */
    
    /* 使能Stage-2 MMU */
    uint64_t hcr = read_hcr_el2();
    hcr |= (1U << 0);    /* VM enable */
    write_hcr_el2(hcr);

    //uart_puts("HCR_EL2 before enter: ");
    //uart_puthex(read_hcr_el2());
    //uart_puts("\n");

    write_tpidr_el2((uint64_t)vcpu_arch);
    
    /* 进入VM */
    armv8_vcpu_enter(vcpu_arch);
    
    /* 正常情况不会返回，若返回则视为致命错误 */
    while (1) { __asm__ volatile("wfi"); }
}

void armv8_vcpu_stop(hp_vcpu_id_t vcpu_id)
{
    struct armv8_vcpu_arch *arch = armv8_get_vcpu_arch(vcpu_id);
    if (arch != NULL) {
        arch->running = false;
    }
}

void armv8_vcpu_save_state(hp_vcpu_id_t vcpu_id)
{
    /* 上下文已由汇编 SAVE_VM_CTX 保存，此处可做额外清理 */
    (void)vcpu_id;
}
