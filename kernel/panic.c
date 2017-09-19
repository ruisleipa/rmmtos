#include "panic.h"

extern void* _end;

void do_panic(char* str,char* file,unsigned int line)
{
	unsigned int* addr = 0x1337;
	int i = 0;

	asm("cli");

	printf("Panic on line %d of \"%s\": %s\n", line, file, str);
asm("hlt");
	printf("Backtrace:\n");

	addr = &addr;
	addr = addr + 1;

	printf("%x %x %x", addr[1], &_end, addr[0]);
	while((i++ < 20 && addr[1] < (unsigned int)&_end) || addr[1] > &addr)
	{
		printf("%x\n", addr[1]);
		addr = &addr[0];

	}



	asm("hlt");
}


void do_trace(char* file,unsigned int line)
{
	printf("Trace on line %d of \"%s\".\n", line, file);
}

