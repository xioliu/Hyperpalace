#ifndef HP_EXCEPTIONS_H
#define HP_EXCEPTIONS_H


/* 异常类型编码（与 vector.S 中的宏对应） */
#define EXC_TYPE_SYNC_LEL_AARCH64    0x00U
#define EXC_TYPE_IRQ_LEL_AARCH64     0x01U
#define EXC_TYPE_FIQ_LEL_AARCH64     0x02U
#define EXC_TYPE_SERR_LEL_AARCH64    0x03U
#define EXC_TYPE_SYNC_CEL_SPX        0x04U
#define EXC_TYPE_IRQ_CEL_SPX         0x05U
#define EXC_TYPE_FIQ_CEL_SPX         0x06U
#define EXC_TYPE_SERR_CEL_SPX        0x07U
#define EXC_TYPE_SYNC_LEL_AARCH32    0x08U
#define EXC_TYPE_IRQ_LEL_AARCH32     0x09U
#define EXC_TYPE_FIQ_LEL_AARCH32     0x0AU
#define EXC_TYPE_SERR_LEL_AARCH32    0x0BU


#ifndef __ASSEMBLY__

/* 异常帧（仅用于EL2自身异常，VM异常由vCPU结构保存） */
typedef struct {
    uint64_t spsr_el2;
    uint64_t elr_el2;
    uint64_t esr_el2;
    uint64_t far_el2;
} hyp_exception_frame_t;

#endif /* !__ASSEMBLY__ */

#endif /* HP_EXCEPTIONS_H */