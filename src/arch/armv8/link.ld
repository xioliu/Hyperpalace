/* link.ld - Hyperpalace ARMv8 链接脚本 */

OUTPUT_ARCH(aarch64)
ENTRY(_start)

MEMORY
{
    RAM (rwx) : ORIGIN = 0x40000000, LENGTH = 128M
}

SECTIONS
{
    /* ========== 只读段 ========== */
    .text : ALIGN(0x1000)
    {
        KEEP(*(.boot.text))        /* 启动代码（含 _start） */
        *(.text .text.*)
        *(.glue_7)
        *(.glue_7t)
    } > RAM

    .rodata : ALIGN(8)
    {
        *(.rodata .rodata.*)
    } > RAM

    /* 异常向量表（必须对齐 2KB） */
    .vectors : ALIGN(2048)
    {
        KEEP(*(.vectors))          /* 若 vector.S 使用 .section .vectors */
        /* 或者如果 vectors 标记在 .text 内，此处可省略 */
    } > RAM

    /* ========== 数据段 ========== */
    .data : ALIGN(16)
    {
        *(.data .data.*)
        /* 辅助 CPU 释放地址表（共享于多核） */
        KEEP(*(.data.cpu_release))
    } > RAM

    /* ========== BSS 段 ========== */
    .bss : ALIGN(16)
    {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;

        /* 主 CPU 栈（64KB） */
        . = ALIGN(16);
        __stack_bottom = .;
        . = . + 0x10000;
        __stack_top = .;

        /* Stage‑2 页表池（4MB） */
        . = ALIGN(0x1000);
        __stage2_pool_start = .;
        . = . + 0x400000;
        __stage2_pool_end = .;
    } > RAM

    /* ========== Hypervisor 保留区域导出符号 ========== */
    /* 这些符号用于 Stage‑2 映射时禁止 VM 访问 Hypervisor 内存 */
    __hyp_text_start = ADDR(.text);
    __hyp_text_end   = ADDR(.text) + SIZEOF(.text);
    __hyp_data_start = ADDR(.data);
    __hyp_data_end   = ADDR(.data) + SIZEOF(.data);
    __hyp_bss_start  = __bss_start;
    __hyp_bss_end    = __bss_end;

    /* ========== 辅助 CPU 栈 ========== */
    /* secondary_stacks 定义在 .bss 中，此处无需额外符号 */
    /* 但可导出其大小供汇编使用 */
    __secondary_stack_size = 0x1000;

    /* 丢弃不需要的段 */
    /DISCARD/ :
    {
        *(.comment)
        *(.note)
        *(.eh_frame)
    }
}