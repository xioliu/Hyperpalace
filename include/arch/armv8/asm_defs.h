#ifndef _ASM_DEFS_H
#define _ASM_DEFS_H

#ifdef __ASSEMBLER__
.equ CPU_STACK_OFF, 320
#else
#define CPU_STACK_OFF 320
#endif
#ifdef __ASSEMBLER__
.equ VCPU_REGS_OFF, 0
#else
#define VCPU_REGS_OFF 0
#endif
#ifdef __ASSEMBLER__
.equ CPU_STACK_SIZE, 4096
#else
#define CPU_STACK_SIZE 4096
#endif

#endif /* _ASM_DEFS_H */
