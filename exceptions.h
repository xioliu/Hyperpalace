#ifndef __EXCEPTION_H
#define __EXCEPTION_H


/* Current EL with SPx */
#define EL2_EXC_IRQ_SPX   (0x12)

/* Lower EL using AArch64 */
#define AARCH64_EXC_SYNC  (0x21)
#define AARCH64_EXC_IRQ   (0x22)

#ifndef __ASM__
typedef struct exception_frame
{
    uint64_t exc_type;
    uint64_t sp_el0;
    uint64_t elr_el1;
    uint64_t spsr_el1;
    uint64_t x0;
    uint64_t x1;
    uint64_t x2;
    uint64_t x3;
    uint64_t x4;
    uint64_t x5;
    uint64_t x6;
    uint64_t x7;
    uint64_t x8;
    uint64_t x9;
    uint64_t x10;
    uint64_t x11;
    uint64_t x12;
    uint64_t x13;
    uint64_t x14;
    uint64_t x15;
    uint64_t x16;
    uint64_t x17;
    uint64_t x18;
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t x29;
    uint64_t x30;
}exception_t;

void common_trap_handler(exception_t *exc);
#endif

#endif
