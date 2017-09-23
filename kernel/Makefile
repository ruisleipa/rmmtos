AS = as86
ASFLAGS =

CC = bcc
CFLAGS = -0 -O86 -ansi -I./

OBJS = 	start.o \
	printf.o \
	int_stub.o \
	int_disp.o \
	uint64.o \
	string.o \
	panic.o \
	io.o \
	mem.o \
	mutex.o \
	task.o \
	gate.o \
	malloc.o \
	syscall.o \
	clock.o \
	bcc/bcc_long.o \
	bcc/bcc_int.o \
	bcc/ldivmod.o \
	fs/fat/fat.o \
	fs/ramfs/ramfs.o \
	fs/devfs/devfs.o \
	fs/directory.o \
	fs/file.o \
	fs/node.o \
	fs/block.o \
	fs/vfs.o \
	char/keyb.o \
	char/screen.o \
	char/serial.o \
	block/floppy.o


all: kernel.bin kernel_sym.bin

kernel_sym.bin: $(OBJS)
	ld86 $(OBJS) -o kernel_sym.bin

kernel.bin: $(OBJS)
	ld86 $(OBJS) -d -M -o kernel.bin > symbols.bin
clean:
	rm -f $(OBJS)
	rm -f kernel.bin
	rm -f kernel_sym.bin

