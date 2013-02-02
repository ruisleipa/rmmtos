CC = bcc
CFLAGS = -0 -O86 -ansi -I./

OBJS = 	init.o \
	memory.o \
	io.o \
	sys.o

all: init.bin

init.bin: $(OBJS)
	ld86 $(OBJS) -d -M -o init.bin

clean:
	rm -f $(OBJS)
	rm -f init.bin
