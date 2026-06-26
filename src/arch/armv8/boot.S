/*
 * boot.S - ARMv8 通用启动代码
 * 适用于所有 ARMv8 平台，平台差异通过弱函数实现
 */

#define __ASM__ 1

.section .boot.text, "ax"

/* 外部符号 */
.extern kmain
.extern vectors
.extern __stack_top

/* 平台可能定义的额外符号 */
.extern secondary_cpu_entry

/*
 * 主入口 _start
 * 所有 CPU 复位后首先执行
 */
.global _start
_start:
    /* 1. 异常允许与向量表 */
    msr  daifclr, #0xf
    ldr  x1, =vectors
    msr  vbar_el2, x1

    /* 2. 设置 AArch64 EL1 */
    mrs  x0, hcr_el2
    orr  x0, x0, #(1 << 31)   /* RW = 1 */
    msr  hcr_el2, x0

    /* 主 CPU 栈初始化 */
    ldr  x1, =__stack_top
    mov  sp, x1
    /* 跳转到 C 入口 main */
    b    main

/*
 * 从核入口（供 PSCI CPU_ON 使用）
 * 该函数在从核启动时首先执行
 */
.global secondary_start
secondary_start:
    /* 1. 直接写 UART 输出字符 'S'，确认从核进入 */
    //ldr x10, =0x09000000          /* UART0_BASE */
    //mov x11, #'S'
    //str w11, [x10]
    msr  daifclr, #0xf
    ldr  x1, =vectors
    msr  vbar_el2, x1

    /* 2. 设置从核自己的栈（使用 secondary_stacks） */
    .extern secondary_stacks
    mrs  x0, mpidr_el1
    and  x0, x0, #0xFF            /* 获取 CPU ID，应为 1 */
    sub  x0, x0, #1               /* 索引从 0 开始 */
    ldr  x1, =secondary_stacks
    mov  x2, #0x1000              /* 栈大小 */
    madd x1, x0, x2, x1           /* x1 = &secondary_stacks[CPU-1] */
    add  sp, x1, #0x1000          /* sp = 栈顶 */

    /* 3. 跳转到 C 函数 secondary_cpu_entry */
    b    secondary_cpu_entry