#define __ASM__ 1
#include "exceptions.h"

.section .text.vectors, "ax"
.global vectors
.balign 2048
vectors:

/* 宏：保存 VM 上下文到当前 vCPU 的 arch 结构 */
.macro SAVE_VM_CTX
    /* 1. 保存原始 x18 到栈上（临时） */
    str x18, [sp, #-16]!

    /* 2. 从 TPIDR_EL2 获取 arch 指针 */
    mrs x18, tpidr_el2
    cbz x18, 1f

    /* 3. 保存 x0-x17（偏移 0x00 - 0x88） */
    stp x0,  x1,  [x18, #16 * 0]
    stp x2,  x3,  [x18, #16 * 1]
    stp x4,  x5,  [x18, #16 * 2]
    stp x6,  x7,  [x18, #16 * 3]
    stp x8,  x9,  [x18, #16 * 4]
    stp x10, x11, [x18, #16 * 5]
    stp x12, x13, [x18, #16 * 6]
    stp x14, x15, [x18, #16 * 7]
    stp x16, x17, [x18, #16 * 8]

    /* 4. 保存 x19-x28（偏移 0x90 - 0xD0） */
    stp x19, x20, [x18, #16 * 9]
    stp x21, x22, [x18, #16 * 10]
    stp x23, x24, [x18, #16 * 11]
    stp x25, x26, [x18, #16 * 12]
    stp x27, x28, [x18, #16 * 13]

    /* 5. 保存 x29, x30（偏移 0xE0） */
    stp x29, x30, [x18, #16 * 14]

    /* 6. 保存原始 x18（偏移 0xF0） */
    ldr x19, [sp]           /* 从栈中取回原始 x18 */
    str x19, [x18, #16 * 15]

    /* 7. 恢复栈 */
    add sp, sp, #16

    /* 8. 保存 sp_el0, sp_el1, elr_el2, spsr_el2（偏移 0xF8-0x110） */
    mrs x0, sp_el0
    mrs x1, sp_el1
    mrs x2, elr_el2
    mrs x3, spsr_el2
    str x0, [x18, #(31 * 8)]      /* sp_el0 @ 248 */
    str x1, [x18, #(32 * 8)]      /* sp_el1 @ 256 */
    str x2, [x18, #(33 * 8)]      /* elr_el2 @ 264 */
    str x3, [x18, #(34 * 8)]      /* spsr_el2 @ 272 */

1:  /* 若 arch 为空则在此死循环（调试时可改为停机） */
.endm

/* 宏：从 arch 恢复 VM 上下文并 eret */
.macro RESTORE_VM_CTX_AND_ERET
    mrs x18, tpidr_el2
    cbz x18, .

    /* 1. 恢复 sp_el0, sp_el1, elr_el2, spsr_el2 */
    ldr x0, [x18, #(31 * 8)]
    ldr x1, [x18, #(32 * 8)]
    ldr x2, [x18, #(33 * 8)]
    ldr x3, [x18, #(34 * 8)]
    msr sp_el0, x0
    msr sp_el1, x1
    msr elr_el2, x2
    msr spsr_el2, x3

    /* 2. 恢复通用寄存器 x0-x28 */
    ldp x0,  x1,  [x18, #16 * 0]
    ldp x2,  x3,  [x18, #16 * 1]
    ldp x4,  x5,  [x18, #16 * 2]
    ldp x6,  x7,  [x18, #16 * 3]
    ldp x8,  x9,  [x18, #16 * 4]
    ldp x10, x11, [x18, #16 * 5]
    ldp x12, x13, [x18, #16 * 6]
    ldp x14, x15, [x18, #16 * 7]
    ldp x16, x17, [x18, #16 * 8]
    ldp x19, x20, [x18, #16 * 9]
    ldp x21, x22, [x18, #16 * 10]
    ldp x23, x24, [x18, #16 * 11]
    ldp x25, x26, [x18, #16 * 12]
    ldp x27, x28, [x18, #16 * 13]
    ldp x29, x30, [x18, #16 * 14]

    /* 3. 恢复 x18（偏移 0xF0） */
    ldr x19, [x18, #16 * 15]
    mov x18, x19

    eret
.endm

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
    eret
.balign 128
curr_el_spx_irq:
    SAVE_VM_CTX
    bl hyp_irq_handler
    RESTORE_VM_CTX_AND_ERET
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
    SAVE_VM_CTX
    mov x0, #0x00
    mov x1, x18       /* 第二个参数：arch 指针（但此时 x18 已被覆盖为基址？注意 x18 在 SAVE 后仍是基址，但 arch 指针已在 tpidr_el2，可以直接读取） */
    bl lower_exception_handler
    RESTORE_VM_CTX_AND_ERET

.balign 128
lower_el_aarch64_irq:
    SAVE_VM_CTX
    mov x0, #0x01
    mov x1, x18
    bl lower_exception_handler
    RESTORE_VM_CTX_AND_ERET

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