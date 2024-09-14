/*
 * boot_plat.S - QEMU virt 平台启动细节
 * 覆盖架构通用文件中的弱函数
 */

.section .boot.text, "ax"

/* 外部引用 */
.extern secondary_stacks
.extern cpu_release_addr

/*
 * 覆盖弱函数：设置辅助 CPU 的专用栈
 * 输入: x0 = CPU ID
 */
.global platform_secondary_stack_setup
platform_secondary_stack_setup:
    ldr  x1, =secondary_stacks
    mov  x2, #0x1000           /* 栈大小 */
    sub  x0, x0, #1            /* 索引 = CPU ID - 1 */
    madd x1, x0, x2, x1        /* x1 = secondary_stacks[CPU-1] */
    add  sp, x1, #0x1000       /* sp = 栈顶 */
    ret

/*
 * 覆盖弱函数：自旋等待主 CPU 释放
 * 输入: x0 = CPU ID
 */
.global platform_secondary_init_and_wait
platform_secondary_init_and_wait:
    /* 保存 x30 (lr) */
    mov  x21, x30

    /* 计算 &cpu_release_addr[CPU] */
    sub  x0, x0, #1            /* 转为 0 基索引 */
    adr  x19, cpu_release_addr
    mov  x20, #8
    madd x19, x0, x20, x19     /* x19 = &cpu_release_addr[CPU] */

1:
    ldr  x20, [x19]
    cbz  x20, 1b               /* 等待非零 */

    /* 主 CPU 已写入释放地址，现在直接返回 */
    ret