#include "hp_types.h"
#include "vm.h"
#include "hp_config.h"
#include "memory.h"
#include "uart.h"
#include "armv8_vm.h"
#include "vtimer.h"

const hp_vm_config_t vm_configs[] = {
    {
        .name = "VM0", .num_cpus = 1, .cpu_mask = (1U << 0), .entry = 0x50000000,
        .num_ram_regions = 1, .ram_regions = {{0x50000000, 0x2000000, HP_MEM_READ|HP_MEM_WRITE|HP_MEM_EXEC}},
        .num_device_regions = 1, .device_regions = {{UART0_BASE, UART0_SIZE, HP_MEM_READ|HP_MEM_WRITE|HP_MEM_DEVICE|HP_MEM_SHAREABLE}},
        .num_irqs = 1,
        .irqs = { VTIMER_IRQ },
    },
    {
        .name = "VM1", .num_cpus = 1, .cpu_mask = (1U << 1), .entry = 0x60000000,
        .num_ram_regions = 1, .ram_regions = {{0x60000000, 0x2000000, HP_MEM_READ|HP_MEM_WRITE|HP_MEM_EXEC}},
        .num_device_regions = 1, .device_regions = {{UART0_BASE, UART0_SIZE, HP_MEM_READ|HP_MEM_WRITE|HP_MEM_DEVICE|HP_MEM_SHAREABLE}},
        .num_irqs = 1,
        .irqs = { VTIMER_IRQ },
    }
};

const uint32_t vm_configs_count = sizeof(vm_configs) / sizeof(vm_configs[0]);