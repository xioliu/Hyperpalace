TARGET	:= minihyper.elf

CROSS = aarch64-none-elf-
CC = ${CROSS}gcc
AS = ${CROSS}as
LD = ${CROSS}ld
OBJDUMP = ${CROSS}objdump

CFLAGS = -mcpu=cortex-a57 -Wall -Wextra -g
ASM_FLAGS = -mcpu=cortex-a57 -g

OBJS = boot.o vector.o main.o init.o aarch64.o exceptions.o psw.o timer.o uart.o gicv3.o

all:$(TARGET)

${TARGET} : ${OBJS}
	${LD} -T link.ld $^ -o $@
	${OBJDUMP} -D minihyper.elf > minihyper.list

%.o : %.S
	${CC} ${CFLAGS} -c $< -o $@

%.o : %.c
	${CC} ${CFLAGS} -c $<

run:
	${MAKE} minihyper.elf

clean:
	rm -f *.o *.elf *.list

