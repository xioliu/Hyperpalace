#define __ASM__ 1
#include "exceptions.h"
#include "asm_defs.h"

.macro VM_EXIT
    /*
     * 此时 SP_EL2 已经指向 &current_vcpu->regs（由vcpu_arch_entry预先设置）
     * 直接把Guest寄存器写入arch_regs结构体，零拷贝
     */
    stp x0,  x1,  [sp, #(8*0)]
    stp x2,  x3,  [sp, #(8*2)]
    stp x4,  x5,  [sp, #(8*4)]
    stp x6,  x7,  [sp, #(8*6)]
    stp x8,  x9,  [sp, #(8*8)]
    stp x10, x11, [sp, #(8*10)]
    stp x12, x13, [sp, #(8*12)]
    stp x14, x15, [sp, #(8*14)]
    stp x16, x17, [sp, #(8*16)]
    stp x18, x19, [sp, #(8*18)]
    stp x20, x21, [sp, #(8*20)]
    stp x22, x23, [sp, #(8*22)]
    stp x24, x25, [sp, #(8*24)]
    stp x26, x27, [sp, #(8*26)]
    stp x28, x29, [sp, #(8*28)]
    str x30,      [sp, #(8*30)]
    mrs x30, sp_el1
    str x30,      [sp, #(8*30)]

    /* 保存硬件自动存入的ELR_EL2和SPSR_EL2 */
    mrs x0, ELR_EL2
    mrs x1, SPSR_EL2
    stp x0, x1, [sp, #(8*32)]

    /* 切换栈之前，先把arch_regs地址保存到x0 */
    mov x0, sp

    /*
     * 切换到CPU专属栈，运行C代码处理异常
     * TPIDR_EL2 永远保存当前CPU的struct cpu指针
     */
    mrs x1, tpidr_el2
    ldr x2, =(CPU_STACK_OFF + CPU_STACK_SIZE)
    add x1, x1, x2
    mov sp, x1
.endm


.section .text.vectors, "ax"
.balign 2048
.global vectors
vectors:
/* ========== 异常向量表 ========== */
.balign 128
curr_el_sp0_sync:
    b .
.balign 128
curr_el_sp0_irq:
    b .
.balign 128
curr_el_sp0_fiq:
    b .
.balign 128
curr_el_sp0_serr:
    b .

.balign 128
curr_el_spx_sync:
    bl handle_hyp_sync
.balign 128
curr_el_spx_irq:
    bl hyp_irq_handler
.balign 128
curr_el_spx_fiq:
    b .
.balign 128
curr_el_spx_serr:
    mov x0, #'S'
    bl uart_putc          /* 可能需要链接到 uart_putc 函数 */
    b .

.balign 128
lower_el_aarch64_sync:
    VM_EXIT
    bl lower_exception_handler
    b armv8_vcpu_enter

.balign 128
lower_el_aarch64_irq:
    VM_EXIT
    bl lower_irq_handler
    b armv8_vcpu_enter

.balign 128
lower_el_aarch64_fiq:
    b .
.balign 128
lower_el_aarch64_serr:
    b .

/* AArch32 部分预留 */
.balign 128
lower_el_aarch32_sync:
    b .
.balign 128
lower_el_aarch32_irq:
    b .
.balign 128
lower_el_aarch32_fiq:
    b .
.balign 128
lower_el_aarch32_serr:
    b .

.section .text, "ax"
.global armv8_vcpu_enter
armv8_vcpu_enter:
    /*
     * 输入：x0 = &armv8_vcpu_arch（调用C函数时传入）
     * 第一步：把SP_EL2设置为&vcpu->regs，这是零拷贝的核心
     */
    mrs x0, tpidr_el2
    add x0, x0, #VCPU_REGS_OFF
    mov sp, x0

    /* 从arch_regs恢复系统寄存器 */
    ldp x0, x1, [sp, #(8*32)]
    msr ELR_EL2, x0
    msr SPSR_EL2, x1

    /* 恢复所有通用寄存器 */
    ldp x0,  x1,  [sp, #(8*0)]
    ldp x2,  x3,  [sp, #(8*2)]
    ldp x4,  x5,  [sp, #(8*4)]
    ldp x6,  x7,  [sp, #(8*6)]
    ldp x8,  x9,  [sp, #(8*8)]
    ldp x10, x11, [sp, #(8*10)]
    ldp x12, x13, [sp, #(8*12)]
    ldp x14, x15, [sp, #(8*14)]
    ldp x16, x17, [sp, #(8*16)]
    ldp x18, x19, [sp, #(8*18)]
    ldp x20, x21, [sp, #(8*20)]
    ldp x22, x23, [sp, #(8*22)]
    ldp x24, x25, [sp, #(8*24)]
    ldp x26, x27, [sp, #(8*26)]
    ldp x28, x29, [sp, #(8*28)]
    ldr x30,      [sp, #(8*31)]
    msr sp_el1, x30
    ldr x30,      [sp, #(8*30)]

    /* 进入Guest，SP_EL2保持为&vcpu->regs */
    eret