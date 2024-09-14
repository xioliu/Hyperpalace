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
#include "armv8_vm_priv.h"
#include "sysregs.h"
#include "string.h"
#include "uart.h"

/* 外部汇编入口 */
extern void armv8_vcpu_enter(struct armv8_vcpu_arch *ctx);

int32_t armv8_vcpu_init(hp_vcpu_id_t vcpu_id)
{
    struct armv8_vcpu_arch *arch = armv8_get_vcpu_arch(vcpu_id);
    if (arch == NULL) {
        return -HP_EINVAL;
    }
    
    (void)memset(arch, 0, sizeof(*arch));
    
    /* 获取VM入口点 */
    hp_vm_id_t vm_id = hp_vcpu_get_vm_id(vcpu_id);
    uint64_t entry = hp_vm_get_entry(vm_id);
    
    /* 设置初始寄存器状态 */
    arch->elr_el2 = entry;
    arch->spsr_el2 = 0x3C5U;  /* EL1h, 异常屏蔽禁用 */
    arch->sp_el1 = entry + 0x100000U;  /* 假设栈在入口后1MB */
    
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
    uint64_t *pgd = (uint64_t *)(uintptr_t)pgd_pa;

    /* 先设置 VTCR_EL2（SL0=1，起始级别 1） */
    uint64_t vtcr = 0;
    vtcr |= (24U << 0);   // T0SZ = 24  →  IPA 为 40 位 (64‑24)
    vtcr |= (1U << 6);    // SL0  = 1   →  对于 4KB 粒度，起始级别为 Level 1
    vtcr |= (2U << 16);   // PS   = 2   →  40‑bit 物理地址
    vtcr |= (3U << 12);   // SH0  = 3   →  Inner Shareable (页表遍历共享域)
    vtcr |= (1U << 10);   // ORGN0 = 1  →  Outer Write‑Back, Read‑Allocate, Write‑Allocate
    vtcr |= (1U << 8);    // IRGN0 = 1  →  Inner Write‑Back, Read‑Allocate, Write‑Allocate
    vtcr |= (1U << 31);   // RES1 = 1   (必须)
    write_vtcr_el2(vtcr);
    __asm__ volatile("isb");

    /* Level 1 索引（2MB 块） */
    uint32_t l1_index = (0x41000000ULL >> 30) & 0x1FF;   // = 1
    uint64_t block = (0x41000000ULL & 0x0000FFFFC0000000ULL)  // 输出地址[47:30]，2MB对齐
               | (1ULL << 0)                              // Block descriptor (bits[1:0] = 01)
               | (1ULL << 10)                             // AF = 1
               | (1ULL << 2)                              // AttrIndx = 1
               | (3ULL << 6)                              // AP[2:1] = 3 (Read/Write)
               | (3ULL << 8);                             // SH[1:0] = 3 (Inner Shareable)
    pgd[l1_index] = block;

    /* 完整 TLB 刷新 */
    __asm__ volatile("dsb sy" ::: "memory");
    __asm__ volatile("tlbi vmalle1is" ::: "memory");
    __asm__ volatile("dsb ish" ::: "memory");
    __asm__ volatile("isb" ::: "memory");

    /* 设置 VTTBR_EL2 (VMID=1) */
    uint64_t vttbr = pgd_pa | (1ULL << 48);
    write_vttbr_el2(vttbr);

    /* 设置 MAIR_EL2（确保 Attr1 为 Normal WBWA） */
    uint64_t mair = 0;
    mair |= (0x00ULL << 0);
    mair |= (0xFFULL << 8);
    write_mair_el2(mair);

    /* 设置系统寄存器 */
    write_elr_el2(0x41000000);          /* Guest 入口 */
    write_spsr_el2(0x3C5);              /* EL1h, DAIF 全开 */
    
    /* 使能Stage-2 MMU */
    uint64_t hcr = read_hcr_el2();
    hcr |= (1U << 0);    /* VM enable */
    write_hcr_el2(hcr);

    /* 使能 SError 并执行 esb，将挂起的 SError 立即触发为同步异常 */
    __asm__ volatile("msr daifclr, #2" ::: "memory");  /* 清除 A 位 */
    __asm__ volatile("isb");
    __asm__ volatile("esb");         /* 如果存在挂起，将触发同步 SError */
    __asm__ volatile("msr daifset, #2" ::: "memory");  /* 重新屏蔽 SError */

    uart_puts("Entering VM...\n");
    
    /* 进入VM */
    armv8_vcpu_enter(vcpu_arch);

    uart_puts("VM returned unexpectedly!\n");
    
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
