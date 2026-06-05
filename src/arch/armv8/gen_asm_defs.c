// 文件：gen_asm_defs.c
#include "hp_types.h"
//#include "vm.h"
#include "armv8_vm.h"

// 标准offsetof宏的纯C实现，不依赖任何库
#define offsetof(type, member) ((size_t)&(((type*)0)->member))

// 核心宏：将C常量转换为汇编.equ指令
#define DEFINE(sym, val) \
    asm volatile("\n.equ " #sym ", %0" : : "i"(val))

#define DEFINE(sym, val) \
    asm volatile("\n.equ " #sym ", %0" : : "i"(val))

int vcpu_defines(void) {
    DEFINE(CPU_STACK_OFF, offsetof(struct armv8_vcpu_arch, stack));
    DEFINE(VCPU_REGS_OFF, offsetof(struct armv8_vcpu_arch, regs));
    DEFINE(CPU_STACK_SIZE, 4096);
    
    return 0;
}