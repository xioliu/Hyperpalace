#ifndef IVC_H
#define IVC_H

#include <stdint.h>

/* IVC 功能号（HVC 调用） */
#define HVC_IVC_SEND  0x10

/* 共享内存基址（假设固定） */
#define IVC_SHM_BASE  0x70000000ULL

/* 门铃中断号（从 64 开始） */
#define IVC_DOORBELL_BASE 64

/* 发送消息到目标 VM */
static inline void ivc_send(uint32_t target_vm, uint32_t doorbell,
                            const void *data, uint32_t len) {
    // 将数据复制到共享内存（暂时忽略长度检查）
    const uint8_t *src = (const uint8_t *)data;
    uint8_t *dst = (uint8_t *)IVC_SHM_BASE;
    for (uint32_t i = 0; i < len; i++) {
        dst[i] = src[i];
    }
    // 调用 HVC 发送门铃
    __asm__ volatile("mov x0, #%[hvc_id]\n"
                     "mov x1, %[target]\n"
                     "mov x2, %[doorbell]\n"
                     "hvc #0\n"
                     :: [hvc_id] "i"(HVC_IVC_SEND),
                        [target] "r"(target_vm),
                        [doorbell] "r"(doorbell)
                     : "x0", "x1", "x2", "memory");
}

#endif