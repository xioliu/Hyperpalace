#ifndef HP_TYPES_H
#define HP_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#define HP_CONFIG_MAX_VMS           4U
#define HP_CONFIG_MAX_PCPUS         8U      /* 支持的最大物理 CPU 数 */
#define HP_CONFIG_MAX_VCPUS         8U      /* 支持的最大虚拟 CPU 数 */
#define HP_CONFIG_MAX_MEM_REGIONS   8U

#define HP_INVALID_VM_ID            0xFFU
#define HP_INVALID_VCPU_ID          0xFFU
#define HP_INVALID_CPU_ID           0xFFU

/* 中断数量上限 */
#define HP_IRQ_MAX                  1020U

/* noreturn 属性 */
#define HP_NORETURN     __attribute__((noreturn))

typedef uint8_t hp_vm_id_t;
typedef uint8_t hp_vcpu_id_t;
typedef uint8_t hp_pcpu_id_t;

typedef enum {
    HP_VM_STATE_INVALID = 0,
    HP_VM_STATE_CREATED,
    HP_VM_STATE_RUNNING
} hp_vm_state_t;

typedef enum {
    HP_VCPU_STATE_INVALID = 0,
    HP_VCPU_STATE_CREATED,
    HP_VCPU_STATE_RUNNING,
    HP_VCPU_STATE_STOPPED
} hp_vcpu_state_t;

typedef uint32_t hp_mem_perm_t;
typedef uint32_t hp_map_flags_t;

#endif