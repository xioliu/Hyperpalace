/*
 * boot_common.S - ARMv8 通用启动代码
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

    /* 3. 确定 CPU ID */
    mrs  x0, mpidr_el1
    and  x0, x0, #0xFF        /* x0 = CPU ID (affinity0) */

    /* 4. 分发：主 CPU 或辅助 CPU */
    cbz  x0, master_cpu

    /* ---------- 辅助 CPU 路径 ---------- */
secondary_cpu:
    /* 调用平台特定的栈初始化（弱函数） */
    bl   platform_secondary_stack_setup
    /* 调用平台特定的启动等待（弱函数） */
    bl   platform_secondary_init_and_wait
    /* 跳转到平台入口或通用 C 入口 */
    b    secondary_cpu_entry

    /* ---------- 主 CPU 路径 ---------- */
master_cpu:
    /* 主 CPU 栈初始化 */
    ldr  x1, =__stack_top
    mov  sp, x1
    /* 跳转到 C 入口 main */
    b    main

    /* 若返回则停机 */
    b    .

/*
 * ========== 弱函数：平台可覆盖 ==========
 */

/* 辅助 CPU 栈设置，默认不操作（平台可能具有独立栈） */
.weak platform_secondary_stack_setup
platform_secondary_stack_setup:
    ret

/* 辅助 CPU 启动等待与初始化，默认返回（平台可能使用自旋表等） */
.weak platform_secondary_init_and_wait
platform_secondary_init_and_wait:
    ret