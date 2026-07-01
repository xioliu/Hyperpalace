.section .text, "ax"
.balign 2048
.globl vectors
vectors:
    .balign 128
    b _sync_el1t
    .balign 128
    b _irq_el1t
    .balign 128
    b .
    .balign 128
    b .
    .balign 128
    b _sync_el1h
    .balign 128
    b irq_handler  // IRQ EL1h
    .balign 128
    b .
    .balign 128
    b .

/* 保存/恢复宏 */
.macro SAVE_CONTEXT
    sub sp, sp, #32*8
    stp x0, x1, [sp, #0*16]
    stp x2, x3, [sp, #1*16]
    stp x4, x5, [sp, #2*16]
    stp x6, x7, [sp, #3*16]
    stp x8, x9, [sp, #4*16]
    stp x10, x11, [sp, #5*16]
    stp x12, x13, [sp, #6*16]
    stp x14, x15, [sp, #7*16]
    stp x16, x17, [sp, #8*16]
    stp x18, x19, [sp, #9*16]
    stp x20, x21, [sp, #10*16]
    stp x22, x23, [sp, #11*16]
    stp x24, x25, [sp, #12*16]
    stp x26, x27, [sp, #13*16]
    stp x28, x29, [sp, #14*16]
    str x30, [sp, #15*16]
.endm

.macro RESTORE_CONTEXT
    ldp x0, x1, [sp, #0*16]
    ldp x2, x3, [sp, #1*16]
    ldp x4, x5, [sp, #2*16]
    ldp x6, x7, [sp, #3*16]
    ldp x8, x9, [sp, #4*16]
    ldp x10, x11, [sp, #5*16]
    ldp x12, x13, [sp, #6*16]
    ldp x14, x15, [sp, #7*16]
    ldp x16, x17, [sp, #8*16]
    ldp x18, x19, [sp, #9*16]
    ldp x20, x21, [sp, #10*16]
    ldp x22, x23, [sp, #11*16]
    ldp x24, x25, [sp, #12*16]
    ldp x26, x27, [sp, #13*16]
    ldp x28, x29, [sp, #14*16]
    ldr x30, [sp, #15*16]
    add sp, sp, #32*8
.endm

_sync_el1t:
    SAVE_CONTEXT
    bl sync_handler_c
    RESTORE_CONTEXT
    eret

_irq_el1t:
    SAVE_CONTEXT
    bl irq_handler_c
    RESTORE_CONTEXT
    eret

_sync_el1h:
    SAVE_CONTEXT
    bl sync_handler_c
    RESTORE_CONTEXT
    eret

_irq_el1h:
    SAVE_CONTEXT
    bl irq_handler_c
    RESTORE_CONTEXT
    eret

/* ---------- 中断处理程序（纯汇编，与汇编版完全一致） ---------- */
irq_handler:
    sub sp, sp, #32*8
    stp x0, x1, [sp, #0*16]
    stp x2, x3, [sp, #1*16]
    stp x4, x5, [sp, #2*16]
    stp x6, x7, [sp, #3*16]
    stp x8, x9, [sp, #4*16]
    stp x10, x11, [sp, #5*16]
    stp x12, x13, [sp, #6*16]
    stp x14, x15, [sp, #7*16]
    stp x16, x17, [sp, #8*16]
    stp x18, x19, [sp, #9*16]
    stp x20, x21, [sp, #10*16]
    stp x22, x23, [sp, #11*16]
    stp x24, x25, [sp, #12*16]
    stp x26, x27, [sp, #13*16]
    stp x28, x29, [sp, #14*16]
    str x30, [sp, #15*16]

    /* 调用 C 函数重设定时器 */
    bl irq_handler_c

    ldp x0, x1, [sp, #0*16]
    ldp x2, x3, [sp, #1*16]
    ldp x4, x5, [sp, #2*16]
    ldp x6, x7, [sp, #3*16]
    ldp x8, x9, [sp, #4*16]
    ldp x10, x11, [sp, #5*16]
    ldp x12, x13, [sp, #6*16]
    ldp x14, x15, [sp, #7*16]
    ldp x16, x17, [sp, #8*16]
    ldp x18, x19, [sp, #9*16]
    ldp x20, x21, [sp, #10*16]
    ldp x22, x23, [sp, #11*16]
    ldp x24, x25, [sp, #12*16]
    ldp x26, x27, [sp, #13*16]
    ldp x28, x29, [sp, #14*16]
    ldr x30, [sp, #15*16]
    add sp, sp, #32*8
    
    eret
