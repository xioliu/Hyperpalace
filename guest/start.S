.section .text.start, "ax"
.globl _guest_start
_guest_start:
    /* 早期打印 'S' */
    ldr x10, =0x09000000
    mov x11, #'S'
    str w11, [x10]

    /* 设置栈指针到 guest_stack 顶部（由链接器定义） */
    .extern guest_stack
    ldr x0, =guest_stack
    add x0, x0, #4096    /* 4KB 栈 */
    mov sp, x0

    /* 跳转到 C 入口 */
    b _guest_start_c