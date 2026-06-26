/* guest/guest.S */
.section .text
.globl _guest_start
_guest_start:
    /* 初始化栈 */
    ldr x0, =0x50010000
    mov sp, x0
    
    /* 设置中断向量表 */
    adr x0, vectors
    msr vbar_el1, x0
    isb
    
    /* 使能IRQ */
    msr daifclr, #2
    
    /* 设置虚拟定时器：1秒后触发 */
    ldr x0, =62500000  // QEMU virt平台默认频率62.5MHz
    mrs x1, cntvct_el0
    add x1, x1, x0
    msr cntv_cval_el0, x1
    
    /* 启用虚拟定时器 */
    mov x0, #1
    msr cntv_ctl_el0, x0
    isb
    
    /* 打印启动信息 */
    adr x0, guest_boot_msg
    bl uart_puts

    //mov x0, #1
    //ldr x1, =msg
    //hvc #0
    
    /* 等待中断 */
loop:
    //wfi
    b loop

/* --------------------------
   Guest UART驱动（纯汇编）
   -------------------------- */
uart_putc:
    /* 输入：x0 = 要打印的字符 */
    ldr x1, =0x09000000  // PL011 UART基地址
uart_putc_wait:
    ldr w2, [x1, #0x18]  // 读取UARTFR寄存器
    tst w2, #(1 << 5)    // 检查TXFF位（发送FIFO满）
    b.ne uart_putc_wait   // 如果满，等待
    str w0, [x1, #0x00]  // 写入数据寄存器
    ret

uart_puts:
    /* 输入：x0 = 字符串指针（以'\0'结尾） */
    stp x19, x30, [sp, #-16]!
    mov x19, x0
uart_puts_loop:
    ldrb w0, [x19], #1
    cbz w0, uart_puts_done
    bl uart_putc
    b uart_puts_loop
uart_puts_done:
    ldp x19, x30, [sp], #16
    ret

/* --------------------------
   中断向量表
   -------------------------- */
.section .vectors, "ax"
.balign 2048
vectors:
    .balign 128
    b .  // sync EL1t
    .balign 128
    b .  // IRQ EL1t
    .balign 128
    b .  // FIQ EL1t
    .balign 128
    b .  // SError EL1t
    .balign 128
    b .  // sync EL1h
    .balign 128
    b irq_handler  // IRQ EL1h
    .balign 128
    b .  // FIQ EL1h
    .balign 128
    b .  // SError EL1h

irq_handler:
    /* 保存上下文 */
    sub sp, sp, #32
    stp x0, x1, [sp, #0]
    stp x2, x30, [sp, #16]
    
    /* 清除虚拟定时器中断 */
    mov x0, #0
    msr cntv_ctl_el0, x0
    isb
    
    /* 打印中断信息 */
    adr x0, timer_irq_msg
    bl uart_puts
    
    /* 重新设置定时器：1秒后再次触发 */
    ldr x0, =62500000
    mrs x1, cntvct_el0
    add x1, x1, x0
    msr cntv_cval_el0, x1
    mov x0, #1
    msr cntv_ctl_el0, x0
    isb
    
    /* 恢复上下文 */
    ldp x0, x1, [sp, #0]
    ldp x2, x30, [sp, #16]
    add sp, sp, #32
    eret

/* --------------------------
   字符串常量
   -------------------------- */
.section .rodata
guest_boot_msg: .asciz "Guest 1 init done\n"
timer_irq_msg:  .asciz "Guest 1 receives an irq\n"
msg: .asciz "Hello from Guest 1 via HVC\n"