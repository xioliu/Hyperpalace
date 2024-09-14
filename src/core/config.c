/*
 * config.c - Devicetree configuration parser for Hyperpalace
 * 
 * Uses libfdt for FDT manipulation.
 * MISRA C:2012 compliant.
 */
#include <hyperpalace/vm.h>
#include <hyperpalace/hp_config.h>
#include <string.h>

/* 静态分配的VM配置存储 */
static hp_vm_config_t g_vm_configs[HP_CONFIG_MAX_VMS];
static uint32_t g_num_vms = 0U;

/* 辅助函数：从fdt属性中读取64位地址（2个cell） */
static int32_t fdt_read_addr_cells(const void *fdt, int nodeoffset,
                                   uint64_t *addr, uint64_t *size)
{
    const uint32_t *prop;
    int len;
    uint32_t addr_cells, size_cells;
    
    if ((fdt == NULL) || (addr == NULL)) {
        return HP_CONFIG_ERR_INVAL;
    }
    
    /* 获取地址/大小cell数 */
    addr_cells = 2U; /* 默认使用2，实际应从父节点读取 */
    size_cells = 2U;
    
    prop = fdt_getprop(fdt, nodeoffset, "reg", &len);
    if (prop == NULL) {
        return HP_CONFIG_ERR_NODATA;
    }
    
    if ((uint32_t)len >= ((addr_cells + size_cells) * sizeof(uint32_t))) {
        /* 解析地址（假设大端？FDT是big-endian） */
        uint64_t a = fdt32_to_cpu(prop[0]);
        a = (a << 32) | fdt32_to_cpu(prop[1]);
        *addr = a;
        
        if (size != NULL) {
            uint64_t s = fdt32_to_cpu(prop[addr_cells]);
            s = (s << 32) | fdt32_to_cpu(prop[addr_cells + 1U]);
            *size = s;
        }
        return HP_CONFIG_OK;
    }
    return HP_CONFIG_ERR_BADFORMAT;
}

/* 解析一个内存区域列表（RAM或DEVICE） */
static int32_t parse_regions(const void *fdt, int vm_node,
                             const char *propname,
                             hp_mem_region_t *regions_out,
                             uint32_t max_regions,
                             uint32_t *num_regions,
                             uint32_t base_flags)
{
    const uint32_t *prop;
    int len;
    uint32_t count;
    uint32_t i;
    
    if ((fdt == NULL) || (regions_out == NULL) || (num_regions == NULL)) {
        return HP_CONFIG_ERR_INVAL;
    }
    
    prop = fdt_getprop(fdt, vm_node, propname, &len);
    if (prop == NULL) {
        *num_regions = 0U;
        return HP_CONFIG_OK; /* 没有此属性不算错误 */
    }
    
    /* 每个区域需要4个cell: base_high, base_low, size_high, size_low */
    count = (uint32_t)len / (4U * sizeof(uint32_t));
    if (count > max_regions) {
        return HP_CONFIG_ERR_NOMEM;
    }
    
    for (i = 0U; i < count; i++) {
        uint64_t base, size;
        base = ((uint64_t)fdt32_to_cpu(prop[i*4U]) << 32) |
                fdt32_to_cpu(prop[i*4U + 1U]);
        size = ((uint64_t)fdt32_to_cpu(prop[i*4U + 2U]) << 32) |
                fdt32_to_cpu(prop[i*4U + 3U]);
        regions_out[i].base = base;
        regions_out[i].size = size;
        regions_out[i].flags = base_flags;
        regions_out[i].irq = 0U; /* 后续通过其他属性设置 */
    }
    *num_regions = count;
    return HP_CONFIG_OK;
}

/* 解析中断属性 */
static int32_t parse_device_interrupts(const void *fdt, int vm_node,
                                       hp_mem_region_t *regions,
                                       uint32_t num_regions)
{
    const uint32_t *prop;
    int len;
    uint32_t i;
    
    if (fdt == NULL) {
        return HP_CONFIG_ERR_INVAL;
    }
    
    prop = fdt_getprop(fdt, vm_node, "device-interrupts", &len);
    if (prop == NULL) {
        /* 没有中断属性，全部设为0 */
        for (i = 0U; i < num_regions; i++) {
            regions[i].irq = 0U;
        }
        return HP_CONFIG_OK;
    }
    
    /* 假设每个中断用2个cell表示 (type, number) */
    for (i = 0U; i < num_regions; i++) {
        if ((uint32_t)len >= (int32_t)((i+1U)*2U*sizeof(uint32_t))) {
            /* 我们只关心中断号（第二个cell） */
            regions[i].irq = fdt32_to_cpu(prop[i*2U + 1U]);
        } else {
            regions[i].irq = 0U;
        }
    }
    return HP_CONFIG_OK;
}

/* 解析CPU列表 */
static int32_t parse_cpu_list(const void *fdt, int vm_node,
                              uint32_t *cpu_array,
                              uint32_t max_cpus,
                              uint32_t *num_cpus)
{
    const uint32_t *prop;
    int len;
    uint32_t i;
    
    if ((fdt == NULL) || (cpu_array == NULL) || (num_cpus == NULL)) {
        return HP_CONFIG_ERR_INVAL;
    }
    
    prop = fdt_getprop(fdt, vm_node, "assigned-cpus", &len);
    if (prop == NULL) {
        *num_cpus = 0U;
        return HP_CONFIG_OK;
    }
    
    *num_cpus = (uint32_t)len / sizeof(uint32_t);
    if (*num_cpus > max_cpus) {
        return HP_CONFIG_ERR_NOMEM;
    }
    
    for (i = 0U; i < *num_cpus; i++) {
        cpu_array[i] = fdt32_to_cpu(prop[i]);
    }
    return HP_CONFIG_OK;
}

/* 解析单个VM节点 */
static int32_t parse_vm_node(const void *fdt, int vm_node, hp_vm_config_t *config)
{
    const char *name;
    const uint32_t *prop;
    int len;
    int32_t ret;
    
    if ((fdt == NULL) || (config == NULL)) {
        return HP_CONFIG_ERR_INVAL;
    }
    
    (void)memset(config, 0, sizeof(*config));
    
    /* 获取VM名称 */
    name = fdt_get_name(fdt, vm_node, NULL);
    if (name != NULL) {
        config->name = name;
    } else {
        config->name = "unknown";
    }
    
    /* 获取vmid（必须） */
    prop = fdt_getprop(fdt, vm_node, "vmid", &len);
    if ((prop == NULL) || (len < (int)sizeof(uint32_t))) {
        return HP_CONFIG_ERR_BADFORMAT;
    }
    config->vmid = fdt32_to_cpu(*prop);
    
    /* 解析CPU分配 */
    ret = parse_cpu_list(fdt, vm_node, config->assigned_cpus,
                         HP_CONFIG_MAX_CPUS_PER_VM, &config->num_cpus);
    if (ret != HP_CONFIG_OK) {
        return ret;
    }
    
    /* 解析RAM区域 */
    ret = parse_regions(fdt, vm_node, "ram-regions",
                        config->ram_regions,
                        HP_CONFIG_MAX_RAM_REGIONS_PER_VM,
                        &config->num_ram_regions,
                        HP_REGION_FLAG_RAM | HP_REGION_FLAG_READ |
                        HP_REGION_FLAG_WRITE | HP_REGION_FLAG_EXEC);
    if (ret != HP_CONFIG_OK) {
        return ret;
    }
    
    /* 解析设备区域 */
    ret = parse_regions(fdt, vm_node, "device-regions",
                        config->device_regions,
                        HP_CONFIG_MAX_DEV_REGIONS_PER_VM,
                        &config->num_device_regions,
                        HP_REGION_FLAG_DEVICE | HP_REGION_FLAG_READ |
                        HP_REGION_FLAG_WRITE);
    if (ret != HP_CONFIG_OK) {
        return ret;
    }
    /* 解析设备中断（如果存在） */
    (void)parse_device_interrupts(fdt, vm_node, config->device_regions,
                                  config->num_device_regions);
    
    /* 解析镜像信息 */
    prop = fdt_getprop(fdt, vm_node, "load-addr", &len);
    if ((prop != NULL) && (len >= (int)(2U * sizeof(uint32_t)))) {
        config->load_addr = ((uint64_t)fdt32_to_cpu(prop[0]) << 32) |
                            fdt32_to_cpu(prop[1]);
    } else {
        config->load_addr = 0ULL;
    }
    
    prop = fdt_getprop(fdt, vm_node, "entry-point", &len);
    if ((prop != NULL) && (len >= (int)(2U * sizeof(uint32_t)))) {
        config->entry_point = ((uint64_t)fdt32_to_cpu(prop[0]) << 32) |
                              fdt32_to_cpu(prop[1]);
    } else {
        config->entry_point = config->load_addr; /* 默认与加载地址相同 */
    }
    
    prop = fdt_getprop(fdt, vm_node, "image-size", &len);
    if ((prop != NULL) && (len >= (int)(2U * sizeof(uint32_t)))) {
        config->image_size = ((uint64_t)fdt32_to_cpu(prop[0]) << 32) |
                             fdt32_to_cpu(prop[1]);
    } else {
        config->image_size = 0ULL;
    }
    
    return HP_CONFIG_OK;
}

/* 公共API实现 */
void hp_config_init(void)
{
    g_num_vms = 0U;
}

int32_t hp_config_parse(const void *fdt_blob,
                        hp_vm_config_t **configs,
                        uint32_t *num_vms)
{
    int root_offset, hyperpalace_offset, vm_node;
    int32_t ret;
    
    if ((fdt_blob == NULL) || (configs == NULL) || (num_vms == NULL)) {
        return HP_CONFIG_ERR_INVAL;
    }
    
    /* 验证FDT头 */
    if (fdt_check_header(fdt_blob) != 0) {
        return HP_CONFIG_ERR_BADFORMAT;
    }
    
    /* 定位/hyperpalace节点 */
    root_offset = fdt_path_offset(fdt_blob, "/");
    hyperpalace_offset = fdt_subnode_offset(fdt_blob, root_offset, "hyperpalace");
    if (hyperpalace_offset < 0) {
        return HP_CONFIG_ERR_NODATA;
    }
    
    g_num_vms = 0U;
    fdt_for_each_subnode(vm_node, fdt_blob, hyperpalace_offset) {
        if (g_num_vms >= HP_CONFIG_MAX_VMS) {
            return HP_CONFIG_ERR_NOMEM;
        }
        
        ret = parse_vm_node(fdt_blob, vm_node, &g_vm_configs[g_num_vms]);
        if (ret != HP_CONFIG_OK) {
            return ret;
        }
        g_num_vms++;
    }
    
    *configs = g_vm_configs;
    *num_vms = g_num_vms;
    return HP_CONFIG_OK;
}

const hp_vm_config_t *hp_config_get_vm(uint32_t vmid)
{
    uint32_t i;
    for (i = 0U; i < g_num_vms; i++) {
        if (g_vm_configs[i].vmid == vmid) {
            return &g_vm_configs[i];
        }
    }
    return NULL;
}