#ifndef HP_MEMORY_H
#define HP_MEMORY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* 内存映射权限位 */
#define HP_MEM_READ         (1U << 0)  /* 可读 */
#define HP_MEM_WRITE        (1U << 1)  /* 可写 */
#define HP_MEM_EXEC         (1U << 2)  /* 可执行 */
#define HP_MEM_DEVICE       (1U << 3)  /* 设备内存 */
#define HP_MEM_CACHEABLE    (1U << 4)  /* 可缓存 */
#define HP_MEM_SHAREABLE    (1U << 5)  /* 可共享 */
#define HP_MEM_SECURE       (1U << 6)  /* 安全内存 */
#define HP_MEM_PRIVILEGED   (1U << 7)  /* 仅Hypervisor访问 */

/* 内存映射标志 */
#define HP_MAP_FIXED        (1U << 0)  /* 固定映射 */
#define HP_MAP_ANONYMOUS    (1U << 1)  /* 匿名映射 */
#define HP_MAP_POPULATE     (1U << 2)  /* 预填充页表 */
#define HP_MAP_LOCKED       (1U << 3)  /* 锁定页面 */
#define HP_MAP_NO_OVERWRITE (1U << 4)  /* 禁止覆盖现有映射 */

/* 页面大小定义 */
#define HP_PAGE_SIZE_4K     0x1000U    /* 4KB */
#define HP_PAGE_SIZE_2M     0x200000U  /* 2MB */
#define HP_PAGE_SIZE_1G     0x40000000U /* 1GB */

/* 内存池配置 */
#define HP_MAX_PAGE_TABLES  64U        /* 最大页表数量 */

/* 物理地址和虚拟地址类型 */
typedef uint64_t hp_paddr_t;
typedef uint64_t hp_vaddr_t;
typedef uint32_t hp_mem_perm_t;
typedef uint32_t hp_map_flags_t;

/* 初始化内存管理子系统 */
void hp_memory_init(void);

/* Stage-2 页表池管理 */
uint64_t hp_stage2_alloc_pgd(void);
void hp_stage2_free_pgd(uint64_t pgd_pa);

/* Stage-2 映射操作 */
int32_t hp_stage2_map(uint64_t pgd_pa, uint64_t guest_pa, uint64_t host_pa,
                      uint64_t size, uint32_t perm);
int32_t hp_stage2_unmap(uint64_t pgd_pa, uint64_t guest_pa, uint64_t size);
int32_t hp_stage2_protect(uint64_t pgd_pa, uint64_t guest_pa, uint64_t size,
                          uint32_t perm);

/* 地址转换辅助函数 */
uint64_t hp_va_to_pa(const void *va);
void *hp_pa_to_va(uint64_t pa);

/* Hypervisor 保留区域检查 */
bool hp_memory_is_hyp_reserved(uint64_t pa, uint64_t size);

#endif /* HP_MEMORY_H */