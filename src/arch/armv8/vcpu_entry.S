.global armv8_vcpu_enter
armv8_vcpu_enter:
    mov     x22, x0                 /* x22 = vcpu_arch 基址 */

    /* 加载 sp_el0, sp_el1 */
    ldr     x0, [x22, #(31*8)]
    ldr     x1, [x22, #(32*8)]
    msr     sp_el0, x0
    msr     sp_el1, x1

    /* 加载 ELR_EL2, SPSR_EL2（从 C 层可能已设置，此处不再加载） */
    /* 我们将从 C 层直接设置，因此注释掉以下两行 */
    // ldr     x2, [x22, #(33*8)]
    // msr     elr_el2, x2
    // ldr     x3, [x22, #(34*8)]
    // msr     spsr_el2, x3

    /* 加载 x0-x21（跳过 x22） */
    ldp     x0,  x1,  [x22, #16*0]
    ldp     x2,  x3,  [x22, #16*1]
    ldp     x4,  x5,  [x22, #16*2]
    ldp     x6,  x7,  [x22, #16*3]
    ldp     x8,  x9,  [x22, #16*4]
    ldp     x10, x11, [x22, #16*5]
    ldp     x12, x13, [x22, #16*6]
    ldp     x14, x15, [x22, #16*7]
    ldp     x16, x17, [x22, #16*8]
    ldp     x18, x19, [x22, #16*9]
    ldp     x20, x21, [x22, #16*10]

    /* 加载 x23-x30 */
    ldp     x23, x24, [x22, #184]
    ldp     x25, x26, [x22, #200]
    ldp     x27, x28, [x22, #216]
    ldp     x29, x30, [x22, #232]

    /* 最后单独加载 x22 */
    ldr     x22, [x22, #176]

    eret