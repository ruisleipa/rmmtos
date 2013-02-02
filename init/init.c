#include "io.h"
#include "memory.h"

#define BUFFER (512*2)

int main()
{
	struct File* floppy = openFile("/floppy0", READ);
	struct File* screen = openFile("/screen", WRITE);
	char* buffer = allocateMemory(BUFFER);
	unsigned int i;
	unsigned char newline = '\n';
	unsigned char dot = '.';

	clearMemory(buffer, BUFFER);

	sys_mount();

	if(!buffer)
	{
		sys_exit(0);
		while(1);
	}

	if(!floppy || !screen)
	{
		sys_exit(1);
		while(1);
	}

	readFile(floppy, buffer, BUFFER);

	for(i = 0; i < BUFFER; i++)
	{
		if(i % 64 == 0 && i != 0)
			writeFile(screen, &newline, 1);

		if(buffer[i] >= ' ')
			writeFile(screen, &buffer[i], 1);
		else
			writeFile(screen, &dot, 1);
	}

	sys_exit(2);

	while(1);
}
