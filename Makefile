# Makefile for Hyperpalace (ARMv8 Type-1 hypervisor)

# 交叉编译器前缀
CROSS_COMPILE ?= /home/ubx1szh/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-
CC      = $(CROSS_COMPILE)gcc
AS      = $(CROSS_COMPILE)as
LD      = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = ${CROSS_COMPILE}objdump

# 目标名称
TARGET  = hyperpalace

# 目录结构
ARCH_DIR     = src/arch/armv8
CORE_DIR     = src/core
PLAT_DIR     = src/platform/virt
INCLUDE_DIRS = include \
               include/hyperpalace \
               include/arch/armv8 \
               $(ARCH_DIR) \
               $(PLAT_DIR)

# 源文件
# 汇编文件
ASM_SRCS  = $(ARCH_DIR)/boot.S \
            $(ARCH_DIR)/vector.S \
            $(PLAT_DIR)/boot_plat.S

# C 源文件
C_SRCS    = main.c \
            $(CORE_DIR)/irq.c \
            $(CORE_DIR)/memory.c \
            $(CORE_DIR)/vm.c \
            $(ARCH_DIR)/armv8_arch_ops.c \
            $(ARCH_DIR)/armv8_mmu.c \
            $(ARCH_DIR)/armv8_vcpu.c \
            $(ARCH_DIR)/armv8_vm.c \
            $(ARCH_DIR)/exceptions.c \
            $(ARCH_DIR)/gicv3.c \
            $(ARCH_DIR)/vgic.c \
            $(ARCH_DIR)/vtimer.c \
            $(PLAT_DIR)/platform.c \
            $(PLAT_DIR)/secondary.c \
            $(PLAT_DIR)/uart.c \
			src/lib/string.c

# 对象文件
ASM_OBJS  = $(ASM_SRCS:.S=.o)
C_OBJS    = $(C_SRCS:.c=.o)
OBJS      = $(ASM_OBJS) $(C_OBJS)

# 链接脚本
LDSCRIPT  = $(ARCH_DIR)/link.ld

# 编译选项
# 架构和CPU
ARCH_FLAGS = -march=armv8-a+nofp -mgeneral-regs-only

# 包含路径
INCLUDES   = $(addprefix -I, $(INCLUDE_DIRS))

# C 编译选项
CFLAGS    = -g -Wall -Wextra -Werror -O2 \
            -ffreestanding -nostdinc -nostdlib \
            -fno-common -fno-builtin \
            $(ARCH_FLAGS) \
            $(INCLUDES) \
            -D__ASSEMBLY__=0

# 汇编选项
ASFLAGS   = $(ARCH_FLAGS) \
            $(INCLUDES) \
            -D__ASSEMBLY__=1 \
			-nostdinc -g

# 链接选项
LDFLAGS   = -T $(LDSCRIPT) -nostdlib -static --gc-sections

# 默认目标
.PHONY: all clean

all: $(TARGET).elf $(TARGET).bin

# 链接
$(TARGET).elf: $(OBJS) $(LDSCRIPT)
	@echo "  LD    $@"
	$(LD) $(LDFLAGS) -o $@ $(OBJS) -Map=$(TARGET).map
	${OBJDUMP} -D $(TARGET).elf > $(TARGET).list

# 生成原始二进制
%.bin: %.elf
	@echo "  OBJCOPY $@"
	$(OBJCOPY) -O binary $< $@

# 编译汇编文件
%.o: %.S
	@echo "  AS    $<"
	$(CC) $(ASFLAGS) -c $< -o $@

# 编译 C 文件
%.o: %.c
	@echo "  CC    $<"
	$(CC) $(CFLAGS) -c $< -o $@

# 清理
clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).bin $(TARGET).map $(TARGET).list
	rm -f include/arch/armv8/asm_defs.h
# 辅助目标：运行 QEMU (virt, 2 cores)
# -machine virtualization=on 用来控制进入EL2，否则默认是EL1。
QEMU     = qemu-system-aarch64
QEMU_OPTS = -M virt,gic-version=3 -cpu cortex-a57 \
            -machine virtualization=on \
            -nographic \
            -smp 2 \
            -m 512M \
            -device loader,addr=0x50000000,file=guest_timer.bin,force-raw=on \
            -kernel $(TARGET).elf

run: all
	$(QEMU) $(QEMU_OPTS)

# 调试运行（等待 gdb 连接）
debug: all
	$(QEMU) $(QEMU_OPTS) -s -S

GUEST_SRC = guest_timer.S
GUEST_ELF = guest_timer.elf
GUEST_BIN = guest_timer.bin
GUEST_LST = guest_timer.list

$(GUEST_BIN): $(GUEST_SRC)
	$(CC) -march=armv8-a -nostdlib -ffreestanding -Ttext=0x50000000 -e _guest_start -o $(GUEST_ELF) $<
	$(OBJCOPY) -O binary $(GUEST_ELF) $@
	${OBJDUMP} -D $(GUEST_ELF) > $(GUEST_LST)

guest: $(GUEST_BIN)

guest_clean:
	rm -f $(GUEST_ELF) $(GUEST_BIN) $(GUEST_LST)

# 生成汇编常量头文件
ASM_DEFS := include/arch/armv8/asm_defs.h
GEN_ASM_DEFS := src/arch/armv8/gen_asm_defs.c

# --------------------------
# 核心：生成同时兼容汇编和C的asm_defs.h
# --------------------------
# 修正：匹配任意空白字符开头的.equ行
# --------------------------
$(ASM_DEFS): $(GEN_ASM_DEFS) include/arch/armv8/armv8_vm.h
	@echo "  GEN     $@"
	@# 1. 编译生成临时汇编文件
	@$(CC) $(CFLAGS) -S -o $@.tmp $<
	@# 2. 核心修正：匹配行首有任意空白字符（空格/制表符）的.equ行
	@grep '^[[:space:]]*\.equ' $@.tmp > $@.tmp2
	@# 3. 去除行首空白字符，转换为纯.equ格式
	@sed -i 's/^[[:space:]]*\.equ/.equ/' $@.tmp2
	@# 4. 转换为双格式：汇编用.equ，C用#define
	@sed -i 's/^\.equ \(.*\), \(.*\)/#ifdef __ASSEMBLER__\n.equ \1, \2\n#else\n#define \1 \2\n#endif/' $@.tmp2
	@# 5. 添加头文件保护
	@echo "#ifndef _ASM_DEFS_H" > $@
	@echo "#define _ASM_DEFS_H" >> $@
	@echo "" >> $@
	@cat $@.tmp2 >> $@
	@echo "" >> $@
	@echo "#endif /* _ASM_DEFS_H */" >> $@
	@# 6. 彻底清理临时文件
	@rm -f $@.tmp $@.tmp2
# 所有目标都依赖asm_defs.h，确保编译顺序正确
$(OBJS): $(ASM_DEFS)