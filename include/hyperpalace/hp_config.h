/*
 * config.h - Hyperpalace Configuration Subsystem
 * 
 * MISRA C:2012 compliant. No dynamic memory.
 */

#ifndef HP_CONFIG_H
#define HP_CONFIG_H

#include <stdint.h>

/* 最大支持VM数量（静态分配） */
#ifndef HP_CONFIG_MAX_VMS
#define HP_CONFIG_MAX_VMS   4U
#endif

/* 单个VM最多支持的内存区域数量 */
#ifndef HP_CONFIG_MAX_RAM_REGIONS_PER_VM
#define HP_CONFIG_MAX_RAM_REGIONS_PER_VM   4U
#endif

/* 单个VM最多支持的设备区域数量 */
#ifndef HP_CONFIG_MAX_DEV_REGIONS_PER_VM
#define HP_CONFIG_MAX_DEV_REGIONS_PER_VM   8U
#endif

/* 单个VM最多支持的CPU数量 */
#ifndef HP_CONFIG_MAX_CPUS_PER_VM
#define HP_CONFIG_MAX_CPUS_PER_VM   4U
#endif

/* 单个VM最多支持的VCPU数量 */
#ifndef HP_CONFIG_MAX_VCPUS_PER_VM
#define HP_CONFIG_MAX_VCPUS_PER_VM   4U
#endif

/* 错误码 */
#define HP_CONFIG_OK              0
#define HP_CONFIG_ERR_NODATA     -1
#define HP_CONFIG_ERR_BADFORMAT  -2
#define HP_CONFIG_ERR_NOMEM      -3
#define HP_CONFIG_ERR_INVAL      -4

/* 内存/外设区域类型标志 */
#define HP_REGION_FLAG_RAM       (1U << 0)
#define HP_REGION_FLAG_DEVICE    (1U << 1)
#define HP_REGION_FLAG_DMA       (1U << 2)
#define HP_REGION_FLAG_READ      (1U << 3)
#define HP_REGION_FLAG_WRITE     (1U << 4)
#define HP_REGION_FLAG_EXEC      (1U << 5)

/*
 * 静态VM配置描述符
 */
typedef struct {
    const char *name;                       /* VM名称（指向设备树字符串） */
    uint32_t vmid;                          /* VM ID */
    uint32_t num_cpus;                      /* 分配的CPU数量 */
    uint32_t assigned_cpus[HP_CONFIG_MAX_CPUS_PER_VM];  /* CPU ID数组 */
    
    uint32_t num_ram_regions;               /* RAM区域数量 */
    hp_mem_region_t ram_regions[HP_CONFIG_MAX_RAM_REGIONS_PER_VM];
    
    uint32_t num_device_regions;            /* 设备区域数量 */
    hp_mem_region_t device_regions[HP_CONFIG_MAX_DEV_REGIONS_PER_VM];
    
    uint64_t load_addr;                     /* 镜像加载地址 */
    uint64_t entry_point;                   /* 入口地址 */
    uint64_t image_size;                    /* 镜像大小 */
} hp_vm_config_t;

/*
 * 解析设备树，填充全局VM配置数组。
 * 
 * @param fdt_blob  指向设备树二进制数据的指针
 * @param configs   输出：指向配置数组的指针（由函数内部静态分配）
 * @param num_vms   输出：解析出的VM数量
 * @return 错误码（HP_CONFIG_OK表示成功）
 */
int32_t hp_config_parse(const void *fdt_blob,
                        hp_vm_config_t **configs,
                        uint32_t *num_vms);

/*
 * 根据VMID获取配置
 */
const hp_vm_config_t *hp_config_get_vm(uint32_t vmid);

/*
 * 初始化配置子系统（目前为空，保留将来扩展）
 */
void hp_config_init(void);

#endif /* HP_CONFIG_H */