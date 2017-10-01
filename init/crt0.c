#asm
	xchg bx, bx
	call _main
.loop:	hlt
	jmp .loop
#endasm

#include "malloc.h"
#include "io.h"
#include "printf.h"
#include "sys.h"

extern void* _end;
int rmmtosMain();
struct File* input = 0;
struct File* output = 0;

void main() {
	malloc_init((void*)&_end, (void*)0xfdff);

	output = openFile("/devices/screen", WRITE);
	input = openFile("/devices/keyboard", READ);

	if(!output || !input) {
		sys_exit(255);
		while(1);
	}

	sys_exit(rmmtosMain());
}


