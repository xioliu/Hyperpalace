
#include "hp_types.h"
#include "vm.h"
#include "exceptions.h"
#include "sysregs.h"
#include "uart.h"
#include "irq.h"
#include "platform.h"
#include "gicv3.h"
#include "armv8_vm.h"
#include "armv8_vm_priv.h"
#include "vtimer.h"
#include "ivc.h"
#include "errno.h"
/* ---------- 保留的函数 ---------- */
#if 0
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
    (void)arch;
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
    uart_puts("sync exc: EC=");
    uart_puthex(ec);
    uart_puts("\n");

    if (ec == 0x01U) {              // WFI/WFE 陷阱
        //arch->elr_el2 += 4U;        // 跳过 WFI
        vtimer_check_inject(arch);  // 检查并注入虚拟定时器
        return;
    }

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
#endif

/* HVC 处理函数 */
static void handle_hvc(struct arch_regs *regs)
{
    uint32_t fn = (uint32_t)regs->x[0];   /* x0 传递功能号 */
    switch (fn) {
        case 0:
            /* 空操作，可用于测试 */
            break;
        case 1:
            /* 控制台输出：x1 为字符串指针 */
            uart_puts((const char *)regs->x[1]);
            break;
        case HVC_IVC_SEND: {
            uint32_t target_vm = (uint32_t)regs->x[1];
            uint32_t doorbell = (uint32_t)regs->x[2];
            hp_vcpu_id_t vcpu_id = hp_vcpu_get_current();
            if (vcpu_id == HP_INVALID_VCPU_ID) {
                regs->x[0] = HP_EINVAL;
                break;
            }
            hp_vm_id_t src_vm = hp_vcpu_get_vm_id(vcpu_id);
            int32_t ret = hp_ivc_send(src_vm, target_vm, doorbell);
            regs->x[0] = (uint64_t)ret;
            break;
        }
        default:
            uart_puts("Unknown HVC call: ");
            uart_puthex(fn);
            uart_puts("\n");
            break;
    }
}

/* 从 vector.S 调用的低异常级别入口 */
void lower_exception_handler(struct arch_regs* regs)
{
    uint64_t esr = read_esr_el2();
    uint64_t ec = (esr >> 26) & 0x3f;
    uint64_t far = read_far_el2();
    uint64_t elr = read_elr_el2();
    uint64_t sp_el1 = read_sysreg(sp_el1);
    uint64_t sp_el0 = read_sysreg(sp_el0);
            uart_puts("EL1 Sync Exception:\n");
            uart_puts("  ESR_EL2: "); uart_puthex(esr);
            uart_puts("  ELR_EL2: "); uart_puthex(elr);
            uart_puts("  FAR_EL2: "); uart_puthex(far);
            uart_puts("  sp_el0: "); uart_puthex(sp_el0);
            uart_puts("  sp_el1: "); uart_puthex(sp_el1);
    //uart_puts("regs: ");
    //uart_puthex((uint64_t)regs);
    uart_puts("\n");
     switch (ec) {
        case 0x1:  // WFI/WFE陷阱
            uart_puts("WFI trapped\n");
            regs->elr_el2 += 4;
            break;
        case 0x16: // HVC 指令
            handle_hvc(regs);
            regs->elr_el2 += 4;
            break;
        case 0x24:  // 阶段2数据中止（来自EL1）

            uart_puts("\nData abort\n");
            while (1);  // 调试用，先死循环
            break;
        default:
            uart_puts("Unhandled exception\n");
            while (1);
            break;
    }

}

void lower_irq_handler(struct arch_regs* regs)
{
    /* 读取中断号 */
    int irq = gicc_iar();
    uart_puts("lower_irq_handler\n");
    
    switch (irq) {
        case VTIMER_IRQ:
            virt_timer_interrupt_handler(regs);
            hp_irq_dispatch(irq);
            break;
        case 25:
            gicv3_maintenance_handler();
            break;
        case 0: // SGI 0，用于唤醒Guest，空处理即可
            break;
        default:
            uart_puts("Unhandled IRQ: ");
            break;
    }
    
    /* 结束中断 */
    gicc_eoir(irq);
    gicc_dir(irq);//必须要有
}

/* EL2 自身的中断处理（直接使用 GICv3 统一入口） */
void hyp_irq_handler(void)
{
    gicv3_handle_irq();
}

void handle_hyp_sync(void)
{
    uint64_t esr = read_esr_el2();
    uint64_t elr = read_elr_el2();
    uint64_t far = read_far_el2();

    // 打印异常信息
    uart_puts("EL2 Sync Exception:\n");
    uart_puts("  ESR_EL2: "); uart_puthex(esr);
    uart_puts("  ELR_EL2: "); uart_puthex(elr);
    uart_puts("  FAR_EL2: "); uart_puthex(far);

    uint32_t ec = (esr >> 26) & 0x3F;
    if (ec == 0x2F) {   // SError interrupt (synchronous after ESB)
        uint64_t elr = read_elr_el2();
        write_elr_el2(elr + 4);      /* 跳过 esb 指令 */
        return;
    }
    /* 其他同步异常，死循环 */
    while (1) { __asm__ volatile("wfi"); }
}
