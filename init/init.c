#include "io.h"
#include "memory.h"
#include "string.h"

#define BUFFER (512*2)

void printstr(struct File* f, char* str);

int main()
{
	struct File* floppy = openFile("/devices/floppy0", READ);
	struct File* screen = openFile("/devices/screen", WRITE);
	char* buffer = allocateMemory(BUFFER);
	unsigned int i;
	unsigned char newline = '\n';
	unsigned char dot = '.';

	printstr(screen, "Hello from init!");

	clearMemory(buffer, BUFFER);

	printstr(screen, "Sleeping.");
	sys_sleep(1000);
	printstr(screen, "Slept for 1 seconds.\n");

        printstr(screen, "Mounting floppy:\n");
	sys_mount("/floppy", "fat", "/devices/floppy0");

	if(!buffer)
	{
		sys_exit(0);
		while(1);
	}

	if(!floppy || !screen)
	{
		sys_exit(1);
		while(1);
	}sys_exit(2);
	while(1);
        printstr(screen, "The following is read from floppy:\n");

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

void printstr(struct File* f, char* str) {
    writeFile(f, str, strlen(str));
}
