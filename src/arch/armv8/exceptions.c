
#include "hp_types.h"
#include "vm.h"
#include "exceptions.h"
#include "sysregs.h"
#include "uart.h"
#include "irq.h"
#include "gicv3.h"
#include "armv8_vm_priv.h"

/* ---------- 保留的函数 ---------- */

/* Stage‑2 缺页处理（待完善） */
static bool handle_stage2_abort(struct armv8_vcpu_arch *arch,
                                uint64_t far, uint64_t esr)
{
    (void)arch;
    (void)esr;
    uart_puts("Stage-2 abort at: ");
    uart_puthex(far);
    uart_puts("\n");
    return false;   /* 目前直接失败，未来可实现按需映射或 MMIO 模拟 */
}

/* HVC 调用处理 */
static void handle_hvc(struct armv8_vcpu_arch *arch)
{
    uint64_t hvc_num = arch->x[0];
    (void)hvc_num;
    uart_puts("HVC called\n");
    /* 跳过 HVC 指令并返回 0 */
    arch->elr_el2 += 4U;
    arch->x[0] = 0;
}

/* ---------- 优化后的函数 ---------- */

/* 处理 VM 的 IRQ（物理中断直通，注入 vGIC） */
static void handle_vm_irq(void)
{
    /* 1. 检查维护中断 */
    uint32_t misr;
    __asm__ volatile("mrs %0, S3_4_C12_C11_1" : "=r"(misr));
    if (misr != 0U) {
        gicv3_maintenance_handler();   /* 调用 GICv3 维护处理 */
        return;
    }

    /* 2. 读取物理中断 ID */
    uint32_t iar = gicv3_read_iar();
    uint32_t irqid = iar & 0x3FFU;
    if (irqid >= 1020U) {
        return;   /* 伪中断 */
    }

    /* 3. 交由统一的中断分发模块处理（内部检查归属并注入 vGIC） */
    hp_irq_dispatch(irqid);

    /* 4. 结束物理中断 */
    gicv3_write_eoir(iar);
    gicv3_write_dir(irqid);
}

/* 系统寄存器陷阱处理 */
static void handle_system_register_trap(struct armv8_vcpu_arch *arch,
                                        uint64_t esr)
{
    (void)arch;
    (void)esr;
    uart_puts("System register trap\n");
    hp_vcpu_stop(hp_vcpu_get_current());
    while (1) { __asm__ volatile("wfi"); }
}

/* 同步异常分发 */
static void handle_vm_sync_exception(struct armv8_vcpu_arch *arch,
                                     uint64_t esr, uint64_t far)
{
    uint32_t ec = (uint32_t)((esr >> 26U) & 0x3FU);

    switch (ec) {
        case 0x24U:   /* Instruction Abort from lower EL */
        case 0x25U:   /* Data Abort from lower EL */
            if (!handle_stage2_abort(arch, far, esr)) {
                goto fatal;
            }
            break;
        case 0x16U:   /* HVC instruction */
            handle_hvc(arch);
            break;
        case 0x18U:   /* Trapped MSR/MRS */
            handle_system_register_trap(arch, esr);
            break;
        default:
            uart_puts("Unknown sync EC: ");
            uart_puthex(ec);
            uart_puts("\n");
            goto fatal;
    }
    return;

fatal:
    uart_puts("Fatal VM exception, stopping vCPU\n");
    hp_vcpu_stop(hp_vcpu_get_current());
    while (1) { __asm__ volatile("wfi"); }
}

/* 从 vector.S 调用的低异常级别入口 */
void lower_exception_handler(uint32_t exc_type, struct armv8_vcpu_arch *arch)
{
    uint64_t esr = read_esr_el2();
    uint64_t far = read_far_el2();

    switch (exc_type) {
        case EXC_TYPE_SYNC_LEL_AARCH64:
            handle_vm_sync_exception(arch, esr, far);
            break;
        case EXC_TYPE_IRQ_LEL_AARCH64:
            handle_vm_irq();
            break;
        default:
            uart_puts("Unhandled exception type\n");
            hp_vcpu_stop(hp_vcpu_get_current());
            while (1) { __asm__ volatile("wfi"); }
    }
}

/* EL2 自身的中断处理（直接使用 GICv3 统一入口） */
void hyp_irq_handler(void)
{
    gicv3_handle_irq();
}

void handle_hyp_sync(void)
{
    uint64_t esr = read_esr_el2();
    uint32_t ec = (esr >> 26) & 0x3F;
    if (ec == 0x2F) {   // SError interrupt (synchronous after ESB)
        uint64_t elr = read_elr_el2();
        write_elr_el2(elr + 4);      /* 跳过 esb 指令 */
        return;
    }
    /* 其他同步异常，死循环 */
    while (1) { __asm__ volatile("wfi"); }
}
