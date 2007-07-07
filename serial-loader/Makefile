CROSS_COMPILE?=arm-softfloat-linux-gnueabi-
CC=${CROSS_COMPILE}gcc
OBJCOPY=${CROSS_COMPILE}objcopy
LDFLAGS=-nostdlib -Wall -pipe -Tquickload.lds -g -O2
CFLAGS=-g -O2

SRCS=quickload.S #quickload.c

all: quickload.sandal

quickload.sandal: quickload.bin
	./bin2sandal.py quickload.bin quickload.sandal

quickload.bin: quickload.elf
	${OBJCOPY} -O binary quickload.elf quickload.bin

quickload.elf: ${SRCS} quickload.lds
	${CC} $(LDFLAGS) ${SRCS} -o quickload.elf

clean:
	rm -f quickload.bin quickload.elf quickload.sandal *.s
