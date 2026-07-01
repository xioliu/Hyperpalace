# 基址由外部传入，默认 0x50000000
BASE_ADDR ?= 0x50000000

# 交叉编译器前缀
CROSS_COMPILE ?= /home/ubx1szh/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-
CC      = $(CROSS_COMPILE)gcc
AS      = $(CROSS_COMPILE)as
LD      = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = ${CROSS_COMPILE}objdump

CFLAGS = -march=armv8-a -nostdlib -ffreestanding -Wall -O2 -g -DBASE_ADDR=$(BASE_ADDR)
LDFLAGS = -nostdlib

SRCS = guest.c uart.c timer.c ivc.c
OBJS = start.o $(SRCS:.c=.o) vectors.o

TARGET = guest.elf
BIN = guest.bin
LST = guest.list

all: $(BIN)

link.ld: link.ld.in
	sed 's/@BASE_ADDR@/$(BASE_ADDR)/g' $< > $@

$(TARGET): $(OBJS) link.ld
	$(LD) $(LDFLAGS) -T link.ld -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

vectors.o: vectors.S
	$(AS) -c -o $@ $<

$(BIN): $(TARGET)
	$(OBJCOPY) -O binary $< $@
	${OBJDUMP} -D $(TARGET) > $(LST)

clean:
	rm -f *.o *.elf *.bin *.list link.ld
