.global _entry
_entry:
    mrs x0, mpidr_el1
    and x0, x0, #3
    cmp x0, #0
    bne .
    
    ldr x1, = vectors
    msr vbar_el1, x1
    adrp x0, stack_top

main_label:
    mov sp, x0
    bl main