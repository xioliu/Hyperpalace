#ifndef HP_IVC_H
#define HP_IVC_H

#include <stdint.h>
#include <stdbool.h>

/* IVC 功能号（HVC 调用） */
#define HVC_IVC_SEND  0x10

#define HP_IVC_MAX_DOORBELLS 8

struct hp_ivc_doorbell {
    uint32_t source_vm;
    uint32_t target_vm;
    uint32_t doorbell_id;
    uint32_t irq_id;
    bool enabled;
};

void hp_ivc_init(void);
int32_t hp_ivc_register(uint32_t src, uint32_t dst, uint32_t id, uint32_t irq);
int32_t hp_ivc_send(uint32_t src, uint32_t dst, uint32_t id);

#endif