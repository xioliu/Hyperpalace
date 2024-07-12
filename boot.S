.global _entry
_entry:
    mrs x0, mpidr_el1
    and x0, x0, #3
    cmp x0, #0
    bne .
    
    ldr x1, = vectors
    msr vbar_el2, x1
    adrp x0, stack_top
    
    mov x1, #1
    msr SPSEL, x1

main_label:
    mov sp, x0
    bl test
    bl main
    
test:
	mrs x0, s3_0_c12_c12_0
	ret
