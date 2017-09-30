CC = bcc
CFLAGS = -0 -O86 -ansi -I./

OBJS = 	bcc/bcc_long.o \
	bcc/bcc_int.o \
	bcc/ldivmod.o \
	memory.o \
	io.o \
	sys.o \
	string.o \
	uint64.o \
	printf.o

all: init.bin shell.bin hello.bin

init.bin: init.o $(OBJS)
	ld86 init.o $(OBJS) -d -M -o init.bin

hello.bin: crt0.o hello.o $(OBJS)
	ld86 crt0.o hello.o $(OBJS) -d -M -o hello.bin

shell.bin: crt0.o shell.o $(OBJS)
	ld86 crt0.o shell.o $(OBJS) -d -M -o shell.bin

clean:
	rm -f $(OBJS)
	rm -f shell.bin
	rm -f init.bin
	rm -f hello.bin
