#define __ASM__     1

#include "exceptions.h"

.section ".boot"

.global vectors
vectors:
/*Each entry is 128 bytes*/
#define VECTOR_ENTRY_ALIGN .align 7

.macro VM_EXIT exc_type
    /*save all regs*/
	stp	x29, x30, [sp, #-16]!
	stp	x27, x28, [sp, #-16]!
	stp	x25, x26, [sp, #-16]!
	stp	x23, x24, [sp, #-16]!
	stp	x21, x22, [sp, #-16]!
	stp	x19, x20, [sp, #-16]!
	stp	x17, x18, [sp, #-16]!
	stp	x15, x16, [sp, #-16]!
	stp	x13, x14, [sp, #-16]!
	stp	x11, x12, [sp, #-16]!
	stp	x9, x10, [sp, #-16]!
	stp	x7, x8, [sp, #-16]!
	stp	x5, x6, [sp, #-16]!
	stp	x3, x4, [sp, #-16]!
	stp	x1, x2, [sp, #-16]!

    /*save spsr_el1*/
    mrs x1, spsr_el1
    stp x1, x0, [sp, #-16]!

    /*save elr_el1 and sp_el0*/
    mrs x1, elr_el1
    stp xzr, x1, [sp, #-16]!

    /*sp_el0 to replace xzr above*/
    mov x1, #(\exc_type)
    mrs x0, sp_el0
    stp x1, x0, [sp, #-8]!
.endm

.macro VM_ENTRY
    /*
    drop exception type
    */
    add sp, sp, #8
    
    /*restore elr_el1 and sp_el0*/
    ldp x0, x1, [sp], #16
    msr sp_el0, x0
    msr elr_el1, x1

    /*restore x0 and spsr_el1 regs*/
    ldp x1, x0, [sp], #16
    msr spsr_el1, x1

    /*restore all regs*/
    ldp x1, x2, [sp], #16
    ldp x3, x4, [sp], #16
    ldp x5, x6, [sp], #16
    ldp x7, x8, [sp], #16
    ldp x9, x10, [sp], #16
    ldp x11, x12, [sp], #16
    ldp x13, x14, [sp], #16
    ldp x15, x16, [sp], #16
    ldp x17, x18, [sp], #16
    ldp x19, x20, [sp], #16
    ldp x21, x22, [sp], #16
    ldp x23, x24, [sp], #16
    ldp x25, x26, [sp], #16
    ldp x27, x28, [sp], #16
    ldp x29, x30, [sp], #16

    eret
    
.endm

/*EL2 with sp0*/
    VECTOR_ENTRY_ALIGN
    b .
    VECTOR_ENTRY_ALIGN
    b .
    VECTOR_ENTRY_ALIGN
    b .
    VECTOR_ENTRY_ALIGN
    b .

/*EL2 with spx*/
    VECTOR_ENTRY_ALIGN
    b .
    VECTOR_ENTRY_ALIGN
    b _curr_el_spx_irq
    VECTOR_ENTRY_ALIGN
    b .
    VECTOR_ENTRY_ALIGN
    b .

/*Lowe El using aarch64*/
    VECTOR_ENTRY_ALIGN
lower_el_aarch64_sync:
    VM_EXIT AARCH64_EXC_SYNC
    b exceptions_handler
    VM_ENTRY
    VECTOR_ENTRY_ALIGN
lower_el_aarch64_irq:
    VM_EXIT AARCH64_EXC_IRQ
    b irq_handler
    VM_ENTRY
    VECTOR_ENTRY_ALIGN
lower_el_aarch64_fiq:
    b .
    VECTOR_ENTRY_ALIGN
lower_el_aarch64_serr:
    b .

/*Lowe El using aarch32*/
    VECTOR_ENTRY_ALIGN
lower_el_aarch32_sync:
    b .
    VECTOR_ENTRY_ALIGN
lower_el_aarch32_irq:
    b .
    VECTOR_ENTRY_ALIGN
lower_el_aarch32_fiq:
    b .
    VECTOR_ENTRY_ALIGN
lower_el_aarch32_serr:
    b .
    
_curr_el_spx_irq:
    VM_EXIT EL2_EXC_IRQ_SPX
    /*x0 is the defaut register accordng to ABI*/
    mov x0, sp
    bl common_trap_handler
    VM_ENTRY

