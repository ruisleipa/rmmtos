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

all: init.bin shell.bin hello.bin type.bin echo.bin showtree.bin present.bin showmem.bin

init.bin: init.o $(OBJS)
	ld86 init.o $(OBJS) -d -M -o init.bin

hello.bin: crt0.o hello.o $(OBJS)
	ld86 crt0.o hello.o $(OBJS) -d -M -o hello.bin

shell.bin: crt0.o shell.o $(OBJS)
	ld86 crt0.o shell.o $(OBJS) -d -M -o shell.bin

type.bin: crt0.o type.o $(OBJS)
	ld86 crt0.o type.o $(OBJS) -d -M -o type.bin

echo.bin: crt0.o echo.o $(OBJS)
	ld86 crt0.o echo.o $(OBJS) -d -M -o echo.bin

showtree.bin: crt0.o showtree.o $(OBJS)
	ld86 crt0.o showtree.o $(OBJS) -d -M -o showtree.bin

present.bin: crt0.o present.o $(OBJS)
	ld86 crt0.o present.o $(OBJS) -d -M -o present.bin

showmem.bin: crt0.o showmem.o $(OBJS)
	ld86 crt0.o showmem.o $(OBJS) -d -M -o showmem.bin


clean:
	rm -f $(OBJS)
	rm -f shell.bin
	rm -f init.bin
	rm -f hello.bin
